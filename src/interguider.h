#ifdef __cplusplus
extern "C" 
{
#endif /* __cplusplus */

int   openOled();
void  closeOled();
void  send2Speaker();
void  sendStr2Oled(char message[]);
void  interGuiderOled(int width,int height,int bpp,int channels,unsigned char *showdata);
char  readOled();



#ifdef __cplusplus
}
#endif /* __cplusplus */
