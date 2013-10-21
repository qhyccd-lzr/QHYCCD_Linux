class QHY21
{
public:
        void initQHY21_regs(void);

	void initQHY21_1024x375(void);

	void initQHY21_1024x750(void);

	void initQHY21_2048x1500(void);

	void CorrectQHY21WH(int *w,int *h);
         
        QHY21(void){}
        ~QHY21(void){}
};
