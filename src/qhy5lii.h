

typedef struct QHY5LIIPara
{
    int QHY5L_PLL_Ratio;
    bool CheckBoxQHY5LIILoneExpMode;
    bool longExpMode;
}QHY5LII;

void   InitQHY5LIIRegs(void);
void   SetQHY5LIIGain(unsigned short gain);
void   W_I2C_MICRON_Address16_OneRegister(unsigned short adr, unsigned short dat);
void   SetExposureTime_QHY5LII(unsigned long i);
void   SetGainMonoQHY5LII(double gain);
void   SetGainColorQHY5LII(double gain, double RG, double BG);
void   CorrectQHY5LIIWH(int *w,int *h);
double GetQHY5LIITemp(void);
double setQHY5LREG_PLL(unsigned char clock);
void   initQHY5LII_1280X960(void);
void   initQHY5LII_QVGA(void);
void   initQHY5LII_SVGA(void);
void   initQHY5LII_VGA(void);
void   initQHY5LII_XGA(void);
void   SetQHY5LIIHDR(bool on);
void   Set14Bit(int i);
void   SWIFT_MSBLSBQHY5LII(unsigned char *ImgData);
void   SetSpeedQHY5LII(int i);

