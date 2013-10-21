class QHY22
{
public:
        void initQHY22_regs(void);

	void initQHY22_768x560(void);

	void initQHY22_1536x1120(void);

	void initQHY22_3072x2240(void);

	void CorrectQHY22WH(int *w,int *h);
         
        QHY22(void){}
        ~QHY22(void){}
};
