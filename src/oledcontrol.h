#ifdef __cplusplus
extern "C" 
{
#endif /* __cplusplus */

int   openoled();
void  closeoled();
void  send2speaker();
void  interGuiderOled(int width,int height,int bpp,int channels,unsigned char *showdata);
char  readoled();


#ifdef __cplusplus
}
#endif /* __cplusplus */
