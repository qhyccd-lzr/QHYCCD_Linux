class IC8300
{
public:
        void initIC8300_regs(void);

	void initIC8300_896x644(void);

	void initIC8300_1792x1287(void);

	void initIC8300_3584x2574(void);

	void CorrectIC8300WH(int *w,int *h);

	void ConvertIC8300DataBIN11(unsigned char *ImgData,int x, int y, unsigned short TopSkipPix);

	void ConvertIC8300DataBIN22(unsigned char *ImgData,int x, int y, unsigned short TopSkipPix);

	void ConvertIC8300DataBIN44(unsigned char *ImgData,int x, int y, unsigned short TopSkipPix);

        void oled(unsigned char buffer[]);

	void send2oled(char message[]);

        void writed(unsigned char value);

        void writec(unsigned char value);
 
        IC8300(void){}
        ~IC8300(void){}
};
