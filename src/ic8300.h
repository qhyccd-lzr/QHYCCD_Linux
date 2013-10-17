
void initQHY9L_896x644(void);

void initQHY9L_1792x1287(void);

void initQHY9L_3584x2574(void);

void CorrectQHY9LWH(int *w,int *h);

void ConvertQHY9LDataBIN11(unsigned char *ImgData,int x, int y, unsigned short TopSkipPix);

void ConvertQHY9LDataBIN22(unsigned char *ImgData,int x, int y, unsigned short TopSkipPix);

void ConvertQHY9LDataBIN44(unsigned char *ImgData,int x, int y, unsigned short TopSkipPix);

void send2oled(char message[]);
