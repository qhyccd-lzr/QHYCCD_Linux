class QHY6
{
public:
        void initQHY6_regs(void);

	void initQHY6_400x298(void);

	void initQHY6_800x596(void);

	void CorrectQHY6WH(int *w,int *h);

        void ConvertQHY6PRODataBIN11(unsigned char *Data);

        void ConvertQHY6PRODataBIN12(unsigned char *Data);

        void ConvertQHY6PRODataBIN22(unsigned char *Data);
         
        QHY6(void){}
        ~QHY6(void){}
};
