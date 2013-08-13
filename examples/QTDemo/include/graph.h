//---------------------------------------------------------------------------

#ifndef GraphH
#define GraphH

#include <Qt>
#include <QColor>
#include <QRect>
#include <QList>
#include <QImage>
#include <QPainter>

//---------------------------------------------------------------------------
#undef INFINITY

#define INFINITY  10000000.0

#define LINE_CACHE_SZ	4096

// line style
enum {
        LS_DOT,
        LS_WIRE,
        LS_SOLID
     };

// marker movement style
enum {
        MS_LOCKED = 0x1,
        MS_2D_VIEW = 0x2,
     };

class CMarkerWrapper;
class CLine;
class CMarker;
class CLabel;


class CGraph
{
	//friend class CMarker;
 public:
  CGraph( QWidget *Wnd,   // Owner window
          int size,         // Number of points in each graph
          QString &Title  // Title of graph
        );
  ~CGraph();

  int    GetSize( void );
  // Lines
  bool   AddLine( QString lineName, QColor lineColor, int style );
  QImage *GetBuffer( void );
  void OnPaint( void );
  bool   SetData( QString lineName, double *buffer );
  double GetYvalue( QString lineName, double x );
  bool   BringToFront( QString lineName );
  bool   DeleteLine( QString lineName );
  bool   ClearLine( QString lineName );
  bool   ShowLine( QString lineName, bool Visible );
  // View
  void ResetView( void );
  void SetViewS( int ScrX, int ScrY );
  void SetView( double Xcenter, double Ycenter );
  void AttachView( CGraph *pSrc );
  void DetachView( void );
  void SetZoomFactors( double kx, double ky );
  void GetPosition( int ScrX, int ScrY, double &x, double &y );
  bool GetScrPosition( double x, double y, int &ScrX, int &ScrY );
  void AddViewShift( int dx, int dy );
  void SetViewShiftDelta( int dx, int dy );
  void SetLineScale( bool scale );
  void SetBackColor( QColor bkcolor );
  // Lense
  void SetLenseParams( int Width, int Height, double ZoomX, double ZoomY );
  void MoveLense( int dataX, int dataY, int ScrX, int ScrY );
  void ShowLense( bool Visible );
  // Markers
  bool AddMarker( QString &markerName, double x, double y, QColor markerColor, QString &text, int markerStyle, CMarkerWrapper *onmovefunc );
  bool SetMarkerWrap( CMarkerWrapper *onmovefunc );
  bool DeleteMarker( QString &markerName );
  bool DeleteGroupOfMarkers( QString &trancName );
  bool ShowMarker( QString &markerName, bool Visible );
  void ShowAllMarkers( bool Visible );
  bool ActivateMarker( int ScrX, int ScrY );
  void DeActivateMarker( void );
  bool MoveMarker( int ScrX, int ScrY );
  bool GetMarkerPos( QString &markerName, double &x, double &y );
  bool SetMarkerPos( QString &markerName, double x, double y );
  bool SetMarkerText( QString &markerName, QString &markerText );
  bool GetMarkerAtPos( QString &markerName, double fromX, double toX, int &iterator, double &x, double &y, bool &IsVisible, bool &IsMarker );
  bool GetMarkerByIdx( int idx, QString &markerName, double &x, double &y, bool &IsVisible, bool &IsMarker );
  bool IsMarkerVisible( QString &markerName, bool &vis );
  bool MarkerExists( QString &markerName );

  bool AddLabel( QString &labelName, double x, double y, QColor labelColor, QString &text );
  bool DeleteLabel( QString &labelName );
  int  GetMarkerCount( void );
  void ShowAllLabels( bool Visible );
  // Rulers
  void ShowRulers( bool Visible );
  void SetRulerFactors( double *xFactors, double *yFactors, int order );
  //CVectInternal ViewCenter;

 protected:
 private:
  class CVectInternal
  {
   public:
   	 CVectInternal() : x(0), y(0) {}
        double x,y;

  };

  QWidget  *Owner;
  QPen 		pen;
  QBrush 	brush;
  QPainter Canvas;
  QImage *Buffer;
  
  CGraph( const CGraph & );
  void 	 Refresh( void );
  int      lineGetNumForName( const QString &name);
  CLine   *lineGetLineForName( const QString &name );
  int      markerGetNumForName( const QString &name);
  CMarker *markerGetMarkerForName( const QString &name );
  CVectInternal GetPosition( int ScrX, int ScrY );
  void   DrawLense( void );
  void   DrawMarkers( void );
  void   DrawRulers( const CVectInternal &vL, const CVectInternal &vR, double kX, double kY );
  void   CalculateParams( void );
  void   RepaintParent( void );
  void   RepaintAttached( void );




  QString Name;

  int    numlines;
  QList <CLine *> LineList;
  QPoint HalfBufferSize;
  QRect ClientRect;
  CVectInternal ViewK;
  CVectInternal AddVect;
  CVectInternal ViewCenter;
  CVectInternal ViewShift, ViewShiftDelta;
  CGraph *pView, *Attached;
  double maxlinedelta;
  double maxlinev, minlinev;
  int    LineSize;
  bool   ScaleLineView;

  bool   NeedRefresh;

  // Colors
  QColor BACK_COLOR;
  QColor LENSE_COLOR;
  QColor MARKER_COLOR;
  QColor LABEL_COLOR;

  // Lense vars.
  bool LenseVisible;
  int  LenseScrX, LenseScrY;
  CVectInternal LenseDataPointer;
  CVectInternal LensePos;
  int  LenseHalfWidth, LenseHalfHeight;
  CVectInternal LenseFactor;

  // Marker vars.
  int    nummarkers;
  QList <CMarker *> MarkerList;
  CMarkerWrapper *func;

  // Ruler vars.
  bool RulersVisible;
  double RulerXfactors[6], RulerYfactors[6];
  double RulerFactorOrder;

};


class CMarkerWrapper
{
public:
	virtual ~CMarkerWrapper() {};

	virtual void onpressfunc( const QString &, double, double, int, int ) = 0;
	virtual void onmovefunc( const QString &, double, double ) = 0;
	virtual void onreleasefunc( const QString &, double, double ) = 0;
};


class CLine
{
	friend class CGraph;
 public:
 private:
      CLine( QString &lineName, int linesize, QColor lineColor, int linestyle );
      ~CLine();

      void Clear( void );
      bool Show( bool Vis );
      bool Cache( int new_sz );

      QString Name;
      int size;
      QColor color;
      int  style;
      bool visible;
      double *data;
      double maxv, minv;
      double delta;
};


class CMarker
{
	friend class CGraph;
 public:
 protected:
      CMarker( const QString &markerName, double markerPosX, double markerPosY, const QColor &markerColor, const QString &markerText, int markerStyle, CMarkerWrapper *onmovefunc );

      CMarker();
      ~CMarker();

      bool Show( bool Vis );
      void SetText( QString markerText);
      bool Set( double x, double y );
      void Get( double &x, double &y );
      void Validate( double xmin, double xmax, double ymin, double ymax );

      QString Name;
      double position_x, position_y;
      QColor color;
      QString text;
      int style;

      bool visible;
      bool active;
      bool ismarker;

      CMarkerWrapper *func;
};

class CLabel : protected CMarker
{
	friend class CGraph;
 public:
 private:
      CLabel( const QString &labelName, double labelPosX, double labelPosY, const QColor &labelColor, const QString &labelText );
      ~CLabel();

};

#endif
