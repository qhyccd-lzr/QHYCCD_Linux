#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <QDir>
#include <QFileDialog>

#include "ccd.h"
#include "utils.h"
#include "sessionswnd.h"


const char *s_err_str[] =
                {
                	"Ok",
                	"End of session",
                	"Invalid parameter",
                	"Path error",
                	""
                };


sessionswnd::sessionswnd(ccd *parent)
    : QDialog(parent), pmain_wnd(parent)
{
	ui.setupUi(this);

	connect( ui.toolButton_Browse, SIGNAL( clicked() ), this, SLOT( onBrowseButtonClick() ) );
	connect( ui.pushButton_Reset, SIGNAL( clicked() ), this, SLOT( onResetButtonClick() ) );
	connect( ui.pushButton_Apply, SIGNAL( clicked() ), this, SLOT( onApplyButtonClick() ) );
}

sessionswnd::~sessionswnd()
{

}


void sessionswnd::showEvent ( QShowEvent * event )
{
	if( event->spontaneous() )
		return;

	fill_interface();
}


void sessionswnd::closeEvent( QCloseEvent * )
{
	if( !pmain_wnd )
		return;

	pmain_wnd->out_info();
}


void sessionswnd::hideEvent( QHideEvent * event )
{
	if( event->spontaneous() )
		return;

	close();
}


void sessionswnd::onBrowseButtonClick()
{
 QString str;
 QFileDialog *pDlg = new QFileDialog(this);
 QStringList fnames;


	pDlg->setAcceptMode( QFileDialog::AcceptOpen );
	pDlg->setWindowTitle( "Select directory" );
	pDlg->setViewMode( QFileDialog::List );
	pDlg->setFileMode( QFileDialog::Directory );
	pDlg->setOptions( QFileDialog::ShowDirsOnly );

	if( pDlg->exec() )
	{
		fnames = pDlg->selectedFiles();
		if( fnames.count() )
		{
			str = fnames.at(0);
			ui.lineEdit_MainPath->setText( str );
		}
	}

	delete pDlg;
}


void sessionswnd::onResetButtonClick()
{
	QString fname;

	if( !pmain_wnd )
		return;

	pmain_wnd->m_sess->reset_cnt();
	pmain_wnd->out_info();
}


void sessionswnd::onApplyButtonClick()
{
	QString pth, dt, obj;
	bool ok;
	int res = S_ERR_PARAM;
	int i_tmp;
	int fr, cnt;


	if( !pmain_wnd )
		return;

	if( (pth = ui.lineEdit_MainPath->text()).isEmpty() )
	{
		u_msg("Main path is empty");
		return;
	}
	if( (dt = ui.lineEdit_Date->text()).isEmpty() )
	{
		u_msg("Date is empty");
		return;
	}
	if( (obj = ui.lineEdit_Object->text()).isEmpty() )
	{
		u_msg("Object is empty");
		return;
	}

	// from
	i_tmp = ui.lineEdit_From->text().toInt( &ok );
	if( !ok )
	{
		u_msg("'From' must be valid integer value");
		return;
	}
	if( i_tmp < 1 || i_tmp > 600 )
	{
		u_msg("'From' must be > 0 and <= 600");
		return;
	}
	fr = i_tmp;
	//count
	i_tmp = ui.lineEdit_Count->text().toInt( &ok );
	if( !ok )
	{
		u_msg("'Count' must be valid integer value");
		return;
	}
	if( i_tmp < 1 || i_tmp > 3600 )
	{
		u_msg("'Count' must be > 0 and <= 3600");
		return;
	}
	cnt = i_tmp;

	res = pmain_wnd->m_sess->set( pth, dt, obj, fr, cnt, true );

	if( res != S_ERR_OK )
	{
		u_msg( "Error: %s", session::errstr(res) );
		return;
	}

	close();
}


void sessionswnd::fill_interface( void )
{
	if( !pmain_wnd )
		return;

	int cntr[S_MAX_IDX];

	pmain_wnd->m_sess->get_counters( cntr, S_MAX_IDX );

	ui.lineEdit_MainPath->setText( pmain_wnd->m_sess->get_path() );
	ui.lineEdit_Date->setText( pmain_wnd->m_sess->get_date() );
	ui.lineEdit_Object->setText( pmain_wnd->m_sess->get_object() );
	ui.lineEdit_From->setText( QString().setNum( cntr[S_FROM_IDX] ) );
	ui.lineEdit_Count->setText( QString().setNum( cntr[S_COUNT_IDX] ) );
}



//-----------------------------------------------------------------------------
session::session( const cam_base_params &pparams ) :
		m_pparams( pparams )
{
 char *ptr;
 time_t t;
 struct tm *now;
 char outdate[200];


	ptr = getenv( "HOME" );
	m_main_path = QString( ptr ) + "/sessions";

	t = time( NULL );
	now = localtime(&t);
	strftime( outdate, sizeof(outdate), "%Y_%m_%d", now );
	m_date = QString( outdate );

	m_object = "m1";	// m - Messier catalogue

	reset();
}


session::~session()
{
}


void session::reset_cnt( void )
{
	m_cur_frame = 0;
}


void session::reset( void )
{
	m_from = 1;
	m_count = 2;
	m_cur_frame = 0;
}


bool session::inc( void )
{
	if( m_cur_frame < m_count )
	{
		m_cur_frame++;
		return (m_cur_frame < m_count);
	}

 return false;
}


int  session::set( QString &pth, QString &dt, QString &obj, int fr, int cnt, bool reset )
{
	QString path;
	int res;

	if( fr <= 0 || cnt < 1 )
		return S_ERR_PARAM;

	path = pth + "/session_" + dt + "/" + obj;

	res = u_mkpath( path.toUtf8().data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
	if( res != 0 )
		return S_ERR_PATH;

	m_main_path = pth;
	m_date = dt;
	m_object = obj;
	m_from = fr;
	m_count = cnt;
	if( reset )
		m_cur_frame = 0;

	return S_ERR_OK;
}


int session::get_fname( QString *fname ) const
{
	QString path;
	double exposition;

	exposition = m_pparams.Exposition;
	path = m_main_path + "/session_" + m_date + "/" + m_object + "/" + QString().setNum((int)(exposition/1000.0)) + "_" + QString().setNum(m_from+m_cur_frame);
	*fname = path;

 return S_ERR_OK;
}


void session::get_counters( int *out, int n ) const
{
	if( out && n <= S_MAX_IDX )
	{
		out[S_FROM_IDX]  = m_from;
		out[S_COUNT_IDX] = m_count;
		out[S_CUR_IDX]   = m_cur_frame;
	}
}


int  session::set_cur( int cur )
{
	if( cur >= 0 && cur < m_count )
	{
		m_cur_frame = cur;
		return S_ERR_OK;
	}
 return S_ERR_PARAM;
}


int  session::check( void )
{
	if( m_cur_frame >= m_count )
		return S_ERR_END;

 return set( m_main_path, m_date, m_object, m_from, m_count, false );
}


const char *session::errstr( int err )
{
	if( err < S_ERR_OK || err >= S_ERR_MAX )
		return NULL;

	return s_err_str[ err ];
}
