/*
 * qhy5liiviewer.c
 * 
 * Copyright 2013 Joaquin Bogado <joaquinbogado@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
 
/**
 * Compile with 
 * gcc -o qhy5lviewer *c -lSDL -lSDL_image -lpthread -lusb -lcfitsio `pkg-config --libs opencv` -DQHY5L_DEBUG
 * for debuging or 
 * gcc -o qhy5lviewer *c -lSDL -lSDL_image -lpthread -lusb -lcfitsio `pkg-config --libs opencv`
 * to not show debug messages
 * **/
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <getopt.h>
#include <fitsio.h>
#include "qhyccd.h"
#include "common.h"
#include "qhy5lii.h"
#include <stdlib.h>

#define OBSERVER "Joaquin Bogado <joaquinbogado at gmail.com>"

void write_fits(void * array, int width, int height, char *fname);
void write_ppm(void * data, int width, int height, char *filename);

struct camdata{
	int w;
	int h;
	int bpp; //bit depth
	int bin; //binmode default 1x1
	int channels;
	int gain;
	int exptime;
	unsigned char * pixels;
} cam;

struct {
	int count;
	int status;
	int write;
	int quit;
} stat;
qhyccd_device **list;
qhyccd_device *device;
qhyccd_device_handle *device_handle;

SDL_Surface * hello;
SDL_Surface * screen;
SDL_Color colors[256];
SDL_Event event;

char fmt[10] = "ppm";
char basename[256] = "image";
char imagename[256];

void (*writefunction)(void *, int, int, char *) = write_ppm;

void processEvents(void){
	while( SDL_PollEvent( &event ) ){
		/* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
		switch( event.type ){
		case SDL_KEYDOWN:
		//case SDL_KEYUP:
			//process online commands
			switch (event.key.keysym.sym){
			//toggle grabe frame
			case SDLK_s:
				stat.write = !stat.write;
				break;
			case SDLK_p:
				if (cam.gain < 1000){
					cam.gain+=10;
					SetGain(cam.gain);
				}
				break;
			case SDLK_o:
				if (cam.gain > 0){
					cam.gain-=10;
					SetGain(cam.gain);
				}
				break;
			case SDLK_l: //increment by 100ms
				if (cam.exptime+100 < 20000000){ //60s
					cam.exptime+=100;
					SetExposeTime(cam.exptime);
				}
				break;
			case SDLK_k: //decrement by 100ms
				if (cam.exptime-100 > 0){
					cam.exptime-=100;
					SetExposeTime(cam.exptime);
				}
				break;
			case SDLK_y: //increment by 10ms
				if (cam.exptime < 20000000){ //60s
					cam.exptime+=10;
					SetExposeTime(cam.exptime);
				}
				break;
			case SDLK_t: //decrement by 10ms
				if (cam.exptime > 0){
					cam.exptime-=10;
					SetExposeTime(cam.exptime);
				}
				break;
			case SDLK_h: //increment by 1000ms
				if (cam.exptime+1000 < 20000000){ //60s
					cam.exptime+=1000;
					SetExposeTime(cam.exptime);
				}
				break;
			case SDLK_g: //decrement by 1000ms
				if (cam.exptime-1000 > 0){
					cam.exptime-=1000;
					SetExposeTime(cam.exptime);
				}
				break;
			//quit viewer
			case SDLK_q:
				stat.quit = 1;
				break;
			}
			break;
		case SDL_QUIT:
			stat.quit = 1;
			break;
		default:
			break;
		}
	}
}

void printerror( int status){
	if (status){
		fits_report_error(stderr, status);
		exit(status); /* terminate the program, returning error status *///HAY QUE VE ESTO!!!
	}
	return;
}
void write_fits(void * array, int width, int height, char *fname ){
	fitsfile *fptr;
	int status;
	long  fpixel, nelements;
	char filename[256] = "!";
	strncat(filename,fname,255);
	int bitpix = BYTE_IMG;
	long naxis = 3;
	long naxes[3];
	naxes[0]=width;
	naxes[1]=height;
	naxes[2]=1;
	status = 0;
	if (fits_create_file(&fptr, filename, &status))
	printerror( status );
	if (fits_create_img(fptr, bitpix, naxis, naxes, &status)) printerror( status );
	fpixel = 1;
	nelements = naxes[0] * naxes[1];
	if (fits_write_img(fptr, TBYTE, fpixel, nelements, array, &status)) printerror( status );
	if (fits_update_key(fptr, TSTRING, "SOFTWARE", "qhy5liitviewer","", &status)) printerror( status );
	if (fits_update_key(fptr, TSTRING, "CAMERA", "QHY5L-II","", &status)) printerror( status );
//	if (fits_update_key(fptr, TLONG, "GAIN", &cam.gain,"", &status)) printerror( status );
//	if (fits_update_key(fptr, TLONG, "EXPOSURE", &cam.exptime, &status)) printerror( status );
//	if (fits_update_key(fptr, TSTRING, "OBSERVER", OBSERVER, "", &status)) printerror( status );
//	if (fits_update_key(fptr, TSTRING, "DATE-OBS",  "date", &status)) printerror( status );
	if (fits_close_file(fptr, &status)) printerror( status );
}
void write_ppm(void * data, int width, int height, char *filename){
	FILE *h = fopen(filename, "w");
#ifdef QHY5L_DEBUG
	printf("writing file %s w x h =  %dx%d\n", filename, width, height);
#endif
	fprintf(h, "P5\n"); //ppm header
	fprintf(h, "%d %d\n", width, height);//ppm header
	fprintf(h, "255\n");//ppm header
	fwrite(data, width*height, 1, h);
	fclose(h);
}
void show_help(char * progname){
	printf("USAGE: qhy5lviewer [options]\n\nOPTIONS:\n");
	printf("  -g | --gain <gain>\n\t\t Sensor Gain<0 - 1000> (default: 100)\n\n");
	printf("  -t | --exposure <exposure>\n\t\t Exposure time in msec (default: 100)\n\n");
	printf("  -f | --file <filename>\n\t\t Output file name to write images (default: image)\n\n");
	printf("  -m | --format <fmt>\n\t\t File type to write (default: FITS, else ppm file will be created.)\n\n");
	printf("  -h | --help\n\t\t Show this message\n\n");
	
	printf("IN-PROGRAM SHORTCUTS:\n");
	printf("  S -> start/stop frame grabbing\n");
	printf("  P -> gain +10\n");
	printf("  O -> gain -10\n");
	printf("  L -> exposure time +100\n");
	printf("  K -> exposure time -100\n");
	printf("  Q -> exit program\n\n");
	
	exit(0);
}

void parse_args(int argc, char **argv){
	
	struct option long_options[] = {
		{"exposure" ,required_argument, NULL, 't'},
		{"gain", required_argument, NULL, 'g'},
		//{"binning", required_argument, NULL, 'b'}, //NOT YET IMPLEMENTED
		//{"width", required_argument, NULL, 'x'},   //Only full resolution
		//{"height", required_argument, NULL, 'y'},  //Small resolutions don't work :(
		{"file", required_argument, NULL, 'o'},
		//{"count", required_argument, NULL, 'c'},   //NOT YET IMPLEMENTED
		{"format", required_argument, NULL, 'f'}, 
		{"help", no_argument , NULL, 'h'},
		{0, 0, 0, 0}
	};
	while (1) {
		char c;
		c = getopt_long (argc, argv, 
					//"t:g:b:x:y:f:c:f:h",
					"t:g:x:y:o:f:h",
					long_options, NULL);
		if(c == EOF)
			break;
		switch(c) {
		case 't':
			cam.exptime = strtol(optarg, NULL, 0);
			break;
		case 'g':
			cam.gain = strtol(optarg, NULL, 0);
			break;
		case 'o':
			strncpy(basename, optarg, 255);
			break;
		case 'f':
			strncpy(fmt, optarg, 10);
			if (strcmp(fmt, "ppm") == 0){ writefunction = write_ppm; }
			else{ writefunction = write_fits; }
			break;
		case 'h':
			show_help(argv[0]);
			break;
		}
	}
}

void check_args(void){
	if (cam.gain < 0  || cam.gain > 1000){
		cam.gain = 100;
	}
	if (cam.exptime < 0){
		cam.exptime = 100;
	}
}

int main(int argc, char **argv){

	parse_args(argc, argv);
	check_args();
	stat.status = OpenCamera();
#ifdef QHY5L_DEBUG
	printf("OpenCamera devolvio 0x%x\n", stat.status);
#endif
	if (stat.status == 0){
		printf("No cameras found\n");
		exit (-1);
	}
	cam.w=1280;
	cam.h=960;
	cam.bpp=8;
	cam.channels=1;
	SetTransferBit(cam.bpp);
//	GetImageFormat(&cam.w,&cam.h,&cam.bpp,&cam.channels);
#ifdef QHY5L_DEBUG
	printf("%dx%d,%dbbp,%dch\n", cam.w, cam.h, cam.bpp, cam.channels);
#endif

	cam.gain = 10;
	cam.exptime = 1000;
	//SDL artifacts initialization...
	int i;
	for(i=0;i<256;i++){
		colors[i].r=i;
		colors[i].g=i;
		colors[i].b=i;
	}
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_SetVideoMode(cam.w, cam.h, 8, SDL_SWSURFACE |SDL_ANYFORMAT);
	if(!screen){
		printf("Couldn't set video mode: %s\n", SDL_GetError());
		exit(-1);
	}
	hello = SDL_SetVideoMode(cam.w, cam.h, 8, SDL_SWSURFACE);
	SDL_SetPalette(hello, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);
	if(!hello){
		printf("Couldn't set video mode: %s\n", SDL_GetError());
		exit(-1);
	}
	//End SDL init
	SetGain(cam.gain);
	SetExposeTime(cam.exptime);
	
	SetSpeed(false);
	SetUSBTraffic(255);//0-255 increase the value will reduce the speed.

	//SetBin(2,2);
//	SetResolution(cam.w, cam.h);
//	GetImageFormat(&cam.w,&cam.h,&cam.bpp,&cam.channels);
#ifdef QHY5L_DEBUG
	printf("%dx%d,%dbbp,%dch\n", cam.w, cam.h, cam.bpp, cam.channels);
#endif
	cam.pixels = malloc(cam.w*cam.h*(cam.bpp/2));
	//cam.pixels = malloc(cam.w*cam.h*4);
	BeginLive();
	while (!stat.quit){
		//process SDL events
		processEvents();
		
		//display an image in the surface.
		GetImageData(cam.w, cam.h, cam.bpp, cam.channels, cam.pixels);
#ifdef QHY5L_DEBUG
	printf("%dx%d,%dbbp,%dch,%dg, %dms\n", cam.w, cam.h, cam.bpp, cam.channels, cam.gain,cam.exptime);
#endif
		SDL_LockSurface(hello);
		hello->pixels = cam.pixels;
		SDL_UnlockSurface(hello);

		if (stat.write){
			sprintf(imagename, "%s%05d.%s", basename, stat.count, fmt);
			printf("Capturing %s\n", imagename);
			writefunction(cam.pixels, cam.w, cam.h, imagename);
			stat.count++;
		}
		if (SDL_BlitSurface(hello, NULL, screen, NULL)){
			printf("%s\n", SDL_GetError());
		}
		SDL_Flip(screen);
		float temp = GetTemp();
		printf("Temperatura = %f\n", temp);
	}
	void CloseCamera();
	return 0;
}

