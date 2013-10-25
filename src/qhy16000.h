class QHY16000
{
public:
        void initQHY16000_regs(void);

	void initQHY16000_4960x560(void);

	void initQHY16000_4960x3328(void);

	void CorrectQHY16000WH(int *w,int *h);
        
        void sendByte2Regs(unsigned char index,unsigned char value);       
 
        void sendWord2Regs(unsigned char index,unsigned short value);

        void setGain16000(unsigned long gain);
   
        void setExpseTime16000(unsigned long time);

        QHY16000(void){}
        ~QHY16000(void){}
};
