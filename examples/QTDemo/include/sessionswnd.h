#ifndef SESSIONSWND_H
#define SESSIONSWND_H

#include <Qt>
#include <QtGui/QDialog>
#include "camera.h"
#include "ui_sessionswnd.h"


enum sess_idx
{
	S_FROM_IDX = 0,
	S_COUNT_IDX,
	S_CUR_IDX,
	S_MAX_IDX
};


enum sess_err
{
	S_ERR_OK = 0,
	S_ERR_END,
	S_ERR_PARAM,
	S_ERR_PATH,
	S_ERR_MAX
};



class ccd;


class session
{
public:
	session( const cam_base_params &pparams );
	~session();

	void reset_cnt( void );
	void reset( void );

	bool inc( void );
	int get_fname( QString *fname ) const;
	void get_counters( int *out, int n ) const;
	int set( QString &pth, QString &dt, QString &obj, int fr, int cnt, bool reset );
	int  set_cur( int cur );
	int  check( void );
	const QString& get_path( void ) const { return m_main_path; }
	const QString& get_date( void ) const { return m_date; }
	const QString& get_object( void ) const { return m_object; }
	static const char *errstr( int err );
private:
	QString m_main_path, m_date, m_object;
	int m_from, m_count, m_cur_frame;

	const cam_base_params &m_pparams;
};


class sessionswnd : public QDialog
{
    Q_OBJECT

public:
    sessionswnd( ccd *parent = 0 );
    ~sessionswnd();

// my slots
protected slots:
	void onBrowseButtonClick();
	void onResetButtonClick();
	void onApplyButtonClick();

protected:
	void showEvent ( QShowEvent * event );
	void closeEvent( QCloseEvent *event );
	void hideEvent( QHideEvent * event );

private:
	ccd *pmain_wnd;
	void fill_interface( void );

private:
    Ui::sessionswndClass ui;
};

#endif // SESSIONSWND_H
