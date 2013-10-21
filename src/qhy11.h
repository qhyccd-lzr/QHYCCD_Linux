class QHY11
{
public:
        void initQHY11_regs(void);

	void initQHY11_1024x680(void);

	void initQHY11_1368x906(void);

	void initQHY11_2048x1360(void);

	void initQHY11_4096x2720(void);

	void CorrectQHY11WH(int *w,int *h);
         
        QHY11(void){}
        ~QHY11(void){}
};
