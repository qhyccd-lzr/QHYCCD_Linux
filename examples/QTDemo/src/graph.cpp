//---------------------------------------------------------------------------
#include <stdlib.h>
#include <math.h>

#include "graph.h"
#include "utils.h"

//---------------------------------------------------------------------------


///////////////// CLine /////////////////////
CLine :: CLine( QString &lineName, int linesize, QColor lineColor, int linestyle )
{
      Name  = lineName;
      size  = linesize;
      color = lineColor;
      style = linestyle;

      visible = true;
      data = new double[size];

      Clear();
}
CLine :: ~CLine()
{
       delete [] data;
}


void CLine :: Clear()
{
       memset(data, 0, sizeof(double)*size);

       maxv = 0;
       minv = 0;
       delta = 0;

}


bool CLine :: Show( bool Vis )
{
     if( visible != Vis )
     {
         visible = Vis;
         return true;
     }
 return false;
}


////////////////////////////////////////////////////////


///////////////// CMarker //////////////////////////////

CMarker :: CMarker( const QString &markerName, double markerPosX, double markerPosY, const QColor &markerColor, const QString &markerText, int markerStyle, CMarkerWrapper *onmovefunc )
{
      Name       = markerName;
      color      = markerColor;
      text       = markerText;
      style      = markerStyle;

      visible = true;
      active  = false;
      ismarker = true;

      func = NULL;

      Set( markerPosX, markerPosY );

      func = onmovefunc; // Little trick to disable function call in Set() body 
}



CMarker :: CMarker()
{
}

CMarker :: ~CMarker()
{
       
}

bool CMarker :: Show( bool Vis )
{
     if( visible != Vis )
     {
         visible = Vis;
         return true;
     }
 return false;
}

void CMarker :: SetText( QString markerText)
{
     text = markerText;
}

bool CMarker :: Set( double x, double y )
{
     if( position_x != x || position_y != y )
     {
         position_x = x;
         position_y = y;

         if( func )
        	 func->onmovefunc( Name, position_x, position_y );

         return true;
     }
 return false;
}

void CMarker :: Get( double &x, double &y )
{
     x = position_x;
     y = position_y;
}


void CMarker :: Validate( double xmin, double xmax, double ymin, double ymax )
{
	if( position_x < xmin )
		position_x = xmin;
	if( position_x > xmax )
		position_x = xmax;
	if( position_y < ymin )
		position_y = ymin;
	if( position_y > ymax )
		position_y = ymax;
}
////////////////////////////////////////////////////////


///////////////// CLabel //////////////////////////////
CLabel :: CLabel( const QString &labelName, double labelPosX, double labelPosY, const QColor &labelColor, const QString &labelText ) :
		CMarker( labelName, labelPosX, labelPosY, labelColor, labelText, 0, NULL )
{
      ismarker = false;
}
CLabel :: ~CLabel()
{
       
}



////////////////////////////////////////////////////////


//////////////////// CGraph ////////////////////////////////////


CGraph :: CGraph( QWidget *Wnd, int linesize, QString &Title )
{

   Owner = Wnd;
   LineSize = linesize;       
   Name = Title;

   numlines = 0;
   //LineList = new TList();
   
   ClientRect = Wnd->rect();
   HalfBufferSize.rx() = ClientRect.width() / 2;
   HalfBufferSize.ry() = ClientRect.height() / 2;
   
   Buffer = new QImage( ClientRect.width(), ClientRect.height(), QImage::Format_RGB32 );
   
    maxlinedelta = 0;
    maxlinev = 0;
    minlinev = 0;

   pView = this;
   Attached = NULL;

   ViewK.x = 1;
   ViewK.y = 1;
   ViewCenter.x = 0;
   ViewCenter.y = 0;
   ViewShift.x = 0;
   ViewShift.y = 0;
   ViewShiftDelta.x = 0;
   ViewShiftDelta.y = 0;

   ScaleLineView = true;

   //Owner->OnPaint = OnPaint;

   // Styles & colors
   BACK_COLOR  = QColor(24,31, 46);//clBlack;
   LENSE_COLOR = QColor(37,57,118);
   MARKER_COLOR = QColor(0,0,255); //clBlue;
   LABEL_COLOR = QColor(11, 83, 45);

   // Lens
   LenseVisible = false;
   LenseScrX = 0;
   LenseScrY = 0;
   LensePos.x = 0;
   LensePos.y = 0;
   LenseDataPointer.x = 0;
   LenseDataPointer.y = 0;
   LenseHalfWidth = 32;
   LenseHalfHeight = 24;
   LenseFactor.x = 1;
   LenseFactor.y = 1;

   // Markers
   nummarkers = 0;
   //MarkerList = new TList();
   func = NULL;

   // Ruler
   RulersVisible = true;
   RulerFactorOrder = 1;
   RulerXfactors[0] = 0;
   RulerXfactors[1] = 1;
   RulerYfactors[0] = 0;
   RulerYfactors[1] = 1;

   ResetView();

   NeedRefresh = false;

}

CGraph :: ~CGraph()
{
 int i;
 CLine *pLine;
 CMarker *pMarker;

    delete Buffer;

    for( i = 0;i < LineList.count();i++ )
    {
         pLine = LineList.at(i);
         delete pLine;
    }
    LineList.clear();
    //delete LineList;


    for( i = 0;i < MarkerList.count();i++ )
    {
         pMarker = MarkerList.at(i);
         delete pMarker;
    }
    MarkerList.clear();
    //delete MarkerList;
    
}

/////////// Service function ///////////
int CGraph :: lineGetNumForName( const QString &name )
{
 CLine *pLine;

    for( int i = 0;i < LineList.count();i++ )
    {
         pLine = LineList.at(i);
         if( pLine->Name.toUpper() == name.toUpper() )
             return i;
    }

 return -1;
}
/////////// Service function ///////////
CLine * CGraph :: lineGetLineForName( const QString &name )
{
 CLine *pLine;

    for( int i = 0;i < LineList.count();i++ )
    {
         pLine = LineList.at(i);
         if( pLine->Name.toUpper() == name.toUpper() )
             return pLine;
    }

 return NULL;
}

int CGraph :: markerGetNumForName( const QString &name )
{
 CMarker *pMarker;

    for( int i = 0;i < pView->MarkerList.count();i++ )
    {
         pMarker = pView->MarkerList.at(i);
         if( pMarker->Name.toUpper() == name.toUpper() )
             return i;
    }

 return -1;
}
/////////// Service function ///////////
CMarker * CGraph :: markerGetMarkerForName( const QString &name )
{
 CMarker *pMarker;

    for( int i = 0;i < pView->MarkerList.count();i++ )
    {
         pMarker = (CMarker *)pView->MarkerList.at(i);
         if( pMarker->Name.toUpper() == name.toUpper() )
             return pMarker;
    }

 return NULL;
}

//////////// CGraph methods /////////////
int  CGraph::GetSize( void )
{
	return LineSize;
}


bool CGraph :: AddLine( QString lineName, QColor lineColor, int style )
{
 CLine *pLine;

     if( lineGetNumForName( lineName ) != -1 )
     {
         u_msg("AddLine: Line with name '%s' already exist", lineName.toAscii().data() );
         return false;
     }

     pLine = new CLine( lineName, LineSize, lineColor, style );
     LineList.append( pLine );
     numlines = LineList.count();

     NeedRefresh = true;

 return true;
}

bool CGraph :: DeleteLine( QString lineName )
{
 CLine *pLine;
 int pos;

     pos = lineGetNumForName( lineName );

     if( pos == -1 )
     {
          u_msg("DeleteLine: No line with name '%s'", lineName.toAscii().data());
          return false;
     }

     pLine = (CLine *)LineList.at(pos);
     delete pLine;
     LineList.removeAt(pos);

     numlines = LineList.count();

     NeedRefresh = true;
     
 return true;
}

bool CGraph :: AddMarker( QString &markerName, double x, double y, QColor markerColor, QString &text, int markerStyle, CMarkerWrapper *onmovefunc )
{
 CMarker *pMarker;


     if( markerGetNumForName( markerName ) != -1 )
     {
         u_msg("AddMarker: Marker with name '%s' already exist", markerName.toAscii().data());
         return false;
     }

     pMarker = new CMarker( markerName, x, y, markerColor, text, markerStyle, onmovefunc );
     pMarker->Validate( 0, LineSize-1, minlinev, maxlinev );
     MarkerList.append( pMarker );
     nummarkers = MarkerList.count();

     NeedRefresh = true;

 return true;
}


bool CGraph::SetMarkerWrap( CMarkerWrapper *onmovefunc )
{
	func = onmovefunc;

	return true;
}


bool CGraph :: DeleteMarker( QString &markerName )
{
 CMarker *pMarker;
 int pos;

     pos = markerGetNumForName( markerName );

     if( pos == -1 )
     {
          u_msg("DeleteMarker: No marker with name '%s'", markerName.toAscii().data());
          return false;
     }

     pMarker = (CMarker *)MarkerList.at(pos);
     delete pMarker;
     MarkerList.removeAt(pos);

     nummarkers = MarkerList.count();

     NeedRefresh = true;
     
 return true;
}

bool CGraph :: AddLabel( QString &labelName, double x, double y, QColor labelColor, QString &text )
{
 CLabel *pLabel;

     if( markerGetNumForName( labelName ) != -1 )
     {
         u_msg("AddMarker: Label with name '%s' already exist", labelName.toAscii().data());
         return false;
     }

     pLabel = new CLabel( labelName, x, y, labelColor, text );
     MarkerList.append( pLabel );
     nummarkers = MarkerList.count();

     NeedRefresh = true;

 return true;
}

bool CGraph :: DeleteLabel( QString &labelName )
{
 CLabel *pLabel;
 int pos;

     pos = markerGetNumForName( labelName );

     if( pos == -1 )
     {
          u_msg("DeleteMarker: No label with name '%s'", labelName.toAscii().data());
          return false;
     }

     pLabel = (CLabel *)MarkerList.at(pos);
     delete pLabel;
     MarkerList.removeAt(pos);;

     nummarkers = MarkerList.count();

     NeedRefresh = true;
     
 return true;
}

bool CGraph :: DeleteGroupOfMarkers( QString &trancName )
{
 CMarker *pMarker;
 QList<CMarker *>::iterator i;
 
 	 for( i = MarkerList.begin();i != MarkerList.end(); )
 	 {
 		  pMarker = *i;
 		  if( pMarker->Name.toUpper() == trancName.toUpper() )
 		  {
 		      delete pMarker;
 		      i = MarkerList.erase(i);
 		  }
 		  else
 			 i++;
 	 }
/*
     for( i = 0;i < MarkerList->Count;i++ )
     {
          pMarker = (CMarker *)MarkerList.at(i);
          if( pMarker->Name.toUpper() == trancName.toUpper() )
          {
              delete pMarker;
              MarkerList->Items[i] = NULL;
          }
     }
     //MarkerList->Pack();
*/
     nummarkers = MarkerList.count();

     NeedRefresh = true;

 return true;
}


//////////////////// Visualizing functions /////////////////////


void CGraph ::  OnPaint( void )
{
 int owd, oht, cwd, cht;
 
 	owd = Owner->width()-4;
 	oht = Owner->height()-4;
 	cwd = ClientRect.width();
 	cht = ClientRect.height();
 
	if( owd != cwd || oht != cht )
	{
		NeedRefresh = true;
		
		owd = owd < 2 ? 2 : owd;
		oht = oht < 2 ? 2 : oht;
		
		delete Buffer;
	    Buffer = new QImage ( owd, oht, QImage::Format_RGB32 );
	             
	    ClientRect.setCoords( 0, 0, owd-1, oht-1 );
	    HalfBufferSize.rx() = owd / 2;
	    HalfBufferSize.ry() = oht / 2;
	}
	
	Canvas.begin( Buffer );
	
	Refresh();
		    
	Canvas.end();
}


QImage *CGraph::GetBuffer( void )
{
	return Buffer;
}


/*************
*
* Main Drawing function
*
**************/
void CGraph :: Refresh( void )
{
 int i,j;
 CLine *pLine;
 double kx, ky;
 int x, y;
 int bottom;
 CVectInternal vl, vr;
 int c1, c2;
 
 QPoint pt;
 QRect  rc;

     if( !NeedRefresh )
         return;

     if( pView != this )
     {
         ViewCenter.x += (pView->ViewCenter.x - ViewCenter.x);
         ViewCenter.y += (pView->ViewCenter.y - ViewCenter.y);
     }

     pen.setStyle( Qt::SolidLine );
     brush.setStyle(Qt::SolidPattern);
     
     brush.setColor(pView->BACK_COLOR);
     Canvas.fillRect(ClientRect, brush);

     vl = GetPosition( 0, 0 );
     vr = GetPosition( ClientRect.width(), 0);

     // Clipping lines
     c1 = (int)vl.x;
     c2 = (int)vr.x;
     if( c1 <= 0 && c2 >= 0 )
         c1 = 0;
     if( c1 < pView->LineSize && c2 >= pView->LineSize )
         c2 = pView->LineSize;
     if( c1 < 0 && c2 < 0 )
         c1 = c2 = -1;
     if( c1 >= pView->LineSize && c2 > pView->LineSize )
         c1 = c2 = -1;

     if( pView->LineSize < 1 )
         return;
     // Rasterizing coefficients
     kx = (double)ClientRect.width()  / (double)pView->LineSize * ViewK.x;
     ky = (double)ClientRect.height() / (pView->maxlinedelta == 0 ? .0001 : pView->maxlinedelta) * ViewK.y;

     for( j = 0;j < pView->numlines;j++ )
     {
          if( c1 == c2 && c1 == -1 )
              break;

          pLine = (CLine *)pView->LineList.at(j);

          if( !pLine->visible )
               continue;


          if( pLine->style == LS_WIRE )
          {
              pen.setColor(pLine->color);
              if( ScaleLineView )
                  pen.setWidth( (int)ViewK.x/2 < 5 ? (int)ViewK.x/2 : 5 );
              else
                  pen.setWidth(1);
              Canvas.setPen(pen);
              pt = QPoint( (c1-ViewCenter.x)*kx + HalfBufferSize.x() + ViewShift.x + ViewShiftDelta.x,
            		  		HalfBufferSize.y() - (int)((pLine->data[c1] - ViewCenter.y) * ky) + ViewShift.y + ViewShiftDelta.y);
          }
          else
          {
        	  pen.setColor(pLine->color);
        	  brush.setColor(pLine->color);
              Canvas.setBrush(brush);
              Canvas.setPen(pen);
          }

          bottom = (int)(ViewCenter.y * ky) + HalfBufferSize.y() + ViewShift.y + ViewShiftDelta.y;

/*
		  // Old rendering
          for( i = c1;i < c2;i++ )
          {
        	  x = (int)((i - ViewCenter.x) * kx) + HalfBufferSize.x() + ViewShift.x + ViewShiftDelta.x;
        	  y = (int)((ViewCenter.y - pLine->data[i]) * ky) + HalfBufferSize.y() + ViewShift.y + ViewShiftDelta.y;

        	  switch( pLine->style )
        	  {
        	  case LS_DOT:
        		  if( ScaleLineView )
        			  rc.setCoords(x, y, x+kx+1, y+kx+1);
        		  else
        			  rc.setCoords(x, y, x+1, y+1);
        		  Canvas.fillRect( rc, brush );
        		  break;
        	  case LS_WIRE:
        		  if( ScaleLineView )
        		  {
        			  Canvas.drawLine( pt.x(), pt.y(), x+kx+1, y );
        			  pt.setX( x+kx+1 );pt.setY(y);
        		  }
        		  else
        		  {
        			  Canvas.drawLine( pt.x(), pt.y(), x+1, y );
        			  pt.setX( x+1 );pt.setY(y);
        		  }
        		  break;
        	  case LS_SOLID:
        		  rc.setCoords(x, y, x+kx+1, bottom);
        		  Canvas.fillRect( rc, brush );
        		  break;
        	  }
          }
*/
          // New rendering
		  int i1, i2;
		  for( i = 0;i < ClientRect.width();i++ )
		  {
			  //i = (int)((ii - ViewCenter.x) * kx) + HalfBufferSize.x() + ViewShift.x + ViewShiftDelta.x;
			  x = i;
			  i1 = ((double)i - ViewShiftDelta.x - ViewShift.x - HalfBufferSize.x()) / kx + ViewCenter.x;
			  if( i1 < 0 )
				  i1 = 0;
			  if( i1 >= pView->LineSize )
				  i1 = pView->LineSize-1;

			  i2 = ((double)(i+1) - ViewShiftDelta.x - ViewShift.x - HalfBufferSize.x()) / kx + ViewCenter.x;
			  if( i2 < 0)
				  i2 = 0;
			  if( i2 >= pView->LineSize )
				  i2 = pView->LineSize-1;

			  double dat_avg = 0;
			  for( int kk = i1;kk < i2;kk++ )
			  {
				  if( fabs(pLine->data[kk]) > dat_avg )
					  dat_avg = fabs(pLine->data[kk]);
			  }
			  if( i1 >= i2 )
				  dat_avg = pLine->data[i1];

			  y = (int)((ViewCenter.y - dat_avg) * ky) + HalfBufferSize.y() + ViewShift.y + ViewShiftDelta.y;

			  switch( pLine->style )
			  {
			  case LS_DOT:
				  if( i1 != i2 )
				  {
					  rc.setCoords(x, y, x+1, y+1);
					  Canvas.fillRect( rc, brush );
				  }
				  break;
			  case LS_WIRE:
				  if( i1 != i2 )
				  {
					  Canvas.drawLine( pt.x(), pt.y(), x, y );
					  pt.setX( x );pt.setY(y);
				  }
				  break;
			  case LS_SOLID:
				  //rc.setCoords(x, y, x, bottom);
				  //Canvas.fillRect( rc, brush );
				  Canvas.drawLine( x, y, x, bottom );
				  break;
			  }
		  }

     }
     pen.setWidth(1);
     Canvas.setPen( pen );

     // Draw markers
     if( pView->nummarkers > 0 )
         DrawMarkers();

     // Draw rulers
     if( RulersVisible )
         DrawRulers( vl, vr, kx, ky );

     // Draw lense
     //if( LenseVisible )
     //    DrawLense();

     RepaintParent();
     RepaintAttached();

     NeedRefresh = false;
}


void CGraph :: DrawRulers( const CVectInternal &vL, const CVectInternal &vR, double kX, double /*kY*/ )
{
 int i, j;
 int x;//, y;
 double step, rval;


 	 pen.setStyle( Qt::SolidLine );
 	 pen.setColor( QColor(128, 128, 128) );
 	 Canvas.setPen( pen );
 	 
     step = (vR.x - vL.x) / 10.0;
     if( step < 0.000001 && step > -0.000001 )
         return;

     int  ns = vL.x / step, ne = vR.x / step + 1;
     QString valTxt;
     for( i = ns;i < ne;i++ )
     {
          x = (int)((i*step - ViewCenter.x) * kX) + HalfBufferSize.x() + ViewShift.x + ViewShiftDelta.x;

          Canvas.drawLine(x,ClientRect.height()-5, x, ClientRect.height() );

          rval = 0;
          for( j = 0;j <= pView->RulerFactorOrder;j++ )
               rval += u_pow( i*step, j )*pView->RulerXfactors[j];

          valTxt.setNum( rval, 'f', 1 );

          Canvas.drawText( x, ClientRect.height()-18, valTxt );

     }
}


void CGraph :: ShowRulers( bool Visible )
{
     if( RulersVisible == Visible )
         return;

     RulersVisible = Visible;

     NeedRefresh = true;
}


void CGraph :: SetRulerFactors( double *xFactors, double *yFactors, int order )
{
 int i;

     if( order > 5 )
         order = 5;
         
     RulerFactorOrder = order;
     
     for( i = 0;i < order+1;i++ )
     {
          RulerXfactors[i] = xFactors[i];
          RulerYfactors[i] = yFactors[i];
     }
}

void CGraph :: RepaintParent()
{
     if( pView != this )
     {
         if( pView->NeedRefresh != NeedRefresh )
         {
             pView->NeedRefresh = NeedRefresh;
             pView->Refresh();
         }
     }
}

void CGraph :: RepaintAttached()
{
     if( Attached != NULL )
     {
         if( Attached->NeedRefresh != NeedRefresh )
         {
             Attached->NeedRefresh = NeedRefresh;
             Attached->Refresh();
         }
     }
}

void CGraph :: SetLineScale( bool scale )
{
     if( ScaleLineView == scale )
         return;

     ScaleLineView = scale;

     NeedRefresh = true;
}

CGraph::CVectInternal CGraph :: GetPosition( int ScrX, int ScrY )
{
 double kx, ky;
 CVectInternal v;

     // Size of item
     kx = (double)ClientRect.width() / (double)pView->LineSize * ViewK.x;
     ky = (double)ClientRect.height() / (pView->maxlinedelta == 0 ? 1 : pView->maxlinedelta) * ViewK.y;
     
     v.x = ((ScrX - HalfBufferSize.x() - ViewShift.x - ViewShiftDelta.x) / kx + ViewCenter.x);
     v.y = ((HalfBufferSize.y() - ScrY + ViewShift.y + ViewShiftDelta.y) / ky + ViewCenter.y);

  return v;
}


bool CGraph :: GetScrPosition( double x, double y, int &ScrX, int &ScrY )
{
 double kx, ky;

     if( pView->LineSize < 1 )
         return false;
         
     kx = (double)ClientRect.width() / (double)pView->LineSize * ViewK.x;
     ky = (double)ClientRect.height() / (pView->maxlinedelta == 0 ? .00001 : pView->maxlinedelta) * ViewK.y;

     ScrX = (int)(x - ViewCenter.x)*kx + HalfBufferSize.x() + ViewShift.x + ViewShiftDelta.x;
     ScrY = (int)(ViewCenter.y - y)*ky + HalfBufferSize.y() + ViewShift.y + ViewShiftDelta.y;

 return true;
}


void CGraph :: GetPosition( int ScrX, int ScrY, double &x, double &y )
{
 CVectInternal v;

     v = GetPosition( ScrX, ScrY );

     x = v.x;
     y = v.y;
}


void CGraph :: ResetView()
{
     SetZoomFactors(1.0, 1.0);

     ViewCenter.x = (double)pView->LineSize / 2.0;
     ViewCenter.y = (pView->maxlinev+pView->minlinev) / 2.0;

     ViewShift.x = 0;
     ViewShift.y = 0;

     ViewShiftDelta.x = 0;
     ViewShiftDelta.y = 0;

     NeedRefresh = true;
}


void CGraph :: AddViewShift( int dx, int dy )
{
     ViewShift.x += dx;
     ViewShift.y += dy;

     NeedRefresh = true;
}


void CGraph :: SetViewShiftDelta( int dx, int dy )
{
     ViewShiftDelta.x = dx;
     ViewShiftDelta.y = dy;

     NeedRefresh = true;
}


void CGraph :: SetViewS( int ScrX, int ScrY )
{
 CVectInternal v;

     v = GetPosition( ScrX, ScrY );

     ViewCenter.x = v.x;
     ViewCenter.y = v.y;

     NeedRefresh = true;
}


void CGraph :: SetView( double Xcenter, double Ycenter )
{
     ViewCenter.x = Xcenter;
     ViewCenter.y = Ycenter;

     NeedRefresh = true;
}


void CGraph :: AttachView( CGraph *pSrc )
{
     pView = pSrc;

     pSrc->Attached = this;
}


void CGraph :: DetachView( )
{
     pView = this;

     pView->Attached = NULL;

}

void CGraph :: SetZoomFactors( double kx, double ky )
{
     if( kx > 0 && kx < 1000 )
     {
         if( ViewK.x != kx )
             NeedRefresh = true;
         ViewK.x = kx;
     }
     else
     {
         u_msg("SetZoomFactors: X zoom factor <= 0 or > 1000");
         return;
     }

     if( ky > 0 && ky < 1000 )
     {
         if( ViewK.y != ky )
             NeedRefresh = true;
         ViewK.y = ky;
     }
     else
     {
         u_msg("SetZoomFactors: Y zoom factor <= 0 or > 1000");
         return;
     }
}

void CGraph :: SetLenseParams( int Width, int Height, double ZoomX, double ZoomY )
{
     if( ZoomX > 0 && ZoomX < 1000 )
         LenseFactor.x = ZoomX;
     else
     {
         u_msg("SetLenseParams: X zoom factor <= 0 or > 1000");
         LenseFactor.x = 1;
     }
     if( ZoomY > 0 && ZoomY < 1000 )
         LenseFactor.y = ZoomY;
     else
     {
         u_msg("SetLenseParams: Y zoom factor <= 0 or > 1000");
         LenseFactor.y = 1;
     }
     if( Width < 16 || Width > 80 || Height < 16 || Height > 80 )
     {
         u_msg("SetLenseParams: Invalid lense size");
         Width = 40;
         Height = 40;
     }

     LenseHalfWidth = Width / 2;
     LenseHalfHeight = Height / 2;

     NeedRefresh = true;
}


void CGraph :: MoveLense( int X, int Y, int ScrX, int ScrY )
{
     if( LenseVisible )
     {
         if( LenseDataPointer.x != X || LenseDataPointer.y != Y ||
             LenseScrX != ScrX || LenseScrY != ScrY
           )
             NeedRefresh = true;

         LenseDataPointer.x = X;
         LenseDataPointer.y = Y;
         LenseScrX = ScrX;
         LenseScrY = ScrY;
     }

}

void CGraph :: ShowLense( bool Visible )
{
     if( LenseVisible != Visible )
     {
         LenseVisible = Visible;

         NeedRefresh = true;
     }
}


void CGraph :: SetBackColor( QColor bkcolor )
{
     BACK_COLOR = bkcolor;
     
     NeedRefresh = true;
}

/*
void CGraph :: DrawLense()
{
 CVectInternal vl, vr;
 QColor bcol;
 int i,j;
 CLine *pLine;
 double kx, ky;
 int x, y;
 int bottom;
 int c1, c2;

     TPoint points[4];
     points[0] = Point(LenseScrX - LenseHalfWidth, LenseScrY - LenseHalfHeight);
     points[1] = Point(LenseScrX + LenseHalfWidth, points[0].y);
     points[2] = Point(points[1].x, LenseScrY + LenseHalfHeight);
     points[3] = Point(points[0].x, points[2].y);
     bcol = Buffer->Canvas.Brush->Color;

     TPenMode oldmode = Buffer->Canvas.Pen->Mode;
     Buffer->Canvas.Pen->Width = 1;

     Buffer->Canvas.Brush->Color = clGray;
     Buffer->Canvas.Pen->Mode = pmMask;
     Buffer->Canvas.Polygon(points, 3);

     //
     Buffer->Canvas.Pen->Mode = pmMerge;
     Buffer->Canvas.Brush->Color = LENSE_COLOR;
     Buffer->Canvas.Polygon(points, 3);

     Buffer->Canvas.Pen->Mode = oldmode;


     LensePos = GetPosition( LenseDataPointer.x, LenseDataPointer.y );
     vl = GetPosition( LenseDataPointer.x - LenseHalfWidth, LenseDataPointer.y - LenseHalfHeight);
     vr = GetPosition( LenseDataPointer.x + LenseHalfWidth, LenseDataPointer.y + LenseHalfHeight);


     // Clipping lines
     c1 = (int)vl.x;
     c2 = (int)vr.x;
     if( c1 <= 0 && c2 >= 0 )
         c1 = 0;
     if( c1 < pView->LineSize && c2 >= pView->LineSize )
         c2 = pView->LineSize;
     if( c1 < 0 && c2 < 0 )
         c1 = c2 = -1;
     if( c1 > pView->LineSize && c2 > pView->LineSize )
         c1 = c2 = -1;
      HRGN MyRgn;

    MyRgn = ::CreateRectRgn(points[0].x, points[0].y ,points[2].x+1, points[2].y+1);
    ::SelectClipRgn(Buffer->Canvas.Handle,MyRgn);



     // Rasterizing coefficients
     kx = (double)ClientRect.Width() / (double)pView->LineSize * ViewK.x * LenseFactor.x;
     ky = (double)ClientRect.Height() / (pView->maxlinedelta == 0 ? .0001 : pView->maxlinedelta) * ViewK.y * LenseFactor.y;

     for( j = 0;j < pView->numlines;j++ )
     {
          if( c1 == c2 && c1 == -1 )
              break;

          pLine = (CLine *)pView->LineList->Items[j];

          if( !pLine->visible )
               continue;



          if( pLine->style == LS_WIRE )
          {
              Buffer->Canvas.Pen->Color = pLine->color;
              if( ScaleLineView )
                  Buffer->Canvas.Pen->Width = kx;
              else
                  Buffer->Canvas.Pen->Width = 1;
              Buffer->Canvas.MoveTo((c1-LensePos.x)*kx + LenseScrX, LenseScrY - (int)((pLine->data[c1] - LensePos.y) * ky));
          }
          else
              Buffer->Canvas.Brush->Color = pLine->color;

          bottom = (int)(LensePos.y * ky) + LenseScrY;

          for( i = c1;i < c2;i++ )
          {
               x = (int)((i - LensePos.x) * kx) + LenseScrX;
               y = (int)((LensePos.y - pLine->data[i]) * ky) + LenseScrY;

               switch( pLine->style )
               {
                case LS_DOT:
                 if( ScaleLineView )
                     Buffer->Canvas.FillRect(TRect(x, y, x+kx+1, y+kx+1));
                 else
                     Buffer->Canvas.FillRect(TRect(x, y, x+1, y+1));
                 break;
                case LS_WIRE:
                 if( ScaleLineView )
                     Buffer->Canvas.LineTo(x+kx+1,y);
                 else
                     Buffer->Canvas.LineTo(x+1,y);
                 break;
                case LS_SOLID:
                 Buffer->Canvas.FillRect(TRect(x, y, x+kx+1, bottom));
                 break;
               }
          }
     }

     Buffer->Canvas.Pen->Mode = oldmode;
     Buffer->Canvas.Brush->Color = bcol;
     Buffer->Canvas.Pen->Width = 1;



     // Last frame
     Buffer->Canvas.Pen->Color = clGray;
     Buffer->Canvas.MoveTo(points[3].x+1, points[3].y-1);
     Buffer->Canvas.LineTo(points[0].x+1, points[0].y+1);
     Buffer->Canvas.LineTo(points[1].x, points[1].y+1);
     Buffer->Canvas.Pen->Color = clSilver;
     Buffer->Canvas.MoveTo(points[3].x, points[3].y);
     Buffer->Canvas.LineTo(points[0].x, points[0].y);
     Buffer->Canvas.LineTo(points[1].x+1, points[1].y);

     Buffer->Canvas.Pen->Color = clGray;
     Buffer->Canvas.MoveTo(points[3].x+1, points[3].y-1);
     Buffer->Canvas.LineTo(points[2].x-1, points[2].y-1);
     Buffer->Canvas.LineTo(points[1].x-1, points[1].y);
     Buffer->Canvas.Pen->Color = clBlack;
     Buffer->Canvas.MoveTo(points[3].x, points[3].y);
     Buffer->Canvas.LineTo(points[2].x, points[2].y);
     Buffer->Canvas.LineTo(points[1].x, points[1].y);



     ::SelectClipRgn(Buffer->Canvas.Handle,NULL);
     ::DeleteObject(MyRgn);


}
*/


void CGraph :: DrawMarkers()
{
 int i;
 CMarker *pMarker;
 double kx, ky;
 int x, y;
 int txtHt = 1;
 QPoint points[4];
 QColor bcol;
 QSize txtSize;


     for( i = 0;i < pView->nummarkers;i++ )
     {
          pMarker = (CMarker *)pView->MarkerList.at(i);

          if( !pMarker->visible )
               continue;

          //u_msg("!");


          // Rasterizing coefficients
          kx = (double)ClientRect.width() / (double)pView->LineSize * ViewK.x;
          ky = (double)ClientRect.height() / (pView->maxlinedelta == 0 ? .00001 : pView->maxlinedelta) * ViewK.y;


          x = (int)((pMarker->position_x - ViewCenter.x) * kx) + HalfBufferSize.x() + ViewShift.x + ViewShiftDelta.x;
          y = (int)((ViewCenter.y - pMarker->position_y) * ky) + HalfBufferSize.y() + ViewShift.y + ViewShiftDelta.y;

          pen.setColor(pMarker->color);
          Canvas.setPen( pen );

          if( pMarker->style & MS_2D_VIEW )
          {
        	  Canvas.drawRect(x-3, y-3, 6, 6 );
          }
          else
          {
        	  y = ClientRect.height();
        	  Canvas.drawLine(x, 0, x, y);
          }

          if( pMarker->text.isEmpty() )
        	  continue;

          // Label frame
          txtSize = Canvas.fontMetrics().size( Qt::TextSingleLine, pMarker->text ); //Canvas.TextExtent(pMarker->text);
          txtSize.rwidth() += 4;


          points[0] = QPoint(x+2, txtHt);
          points[1] = QPoint(points[0].x()+txtSize.width(), txtHt);
          if(points[1].x() > ClientRect.width())
          {
             points[0].rx() = x-2-txtSize.width();
             points[1].rx() = x-2;
          }
          points[2] = QPoint(points[1].x(), txtHt+txtSize.height());
          points[3] = QPoint(points[0].x(), txtHt+txtSize.height());
          bcol = Canvas.brush().color();

          pen.setWidth(1);
          pen.setStyle(Qt::NoPen);
          Canvas.setPen(pen);

          // Use transparency
          if( pMarker->ismarker )
              brush.setColor( QColor(MARKER_COLOR.red(), MARKER_COLOR.green(), MARKER_COLOR.blue(), 128) );
          else
        	  brush.setColor( QColor(LABEL_COLOR.red(), LABEL_COLOR.green(), LABEL_COLOR.blue(), 128) );
          Canvas.setBrush(brush);
          
          
          Canvas.drawPolygon(points, 4);
          
          
          
          pen.setStyle(Qt::SolidLine);
          pen.setColor( pMarker->color );
          Canvas.setPen(pen);
          
          Canvas.drawText( points[0].x()+2, points[0].y()+Canvas.fontMetrics().ascent()+1, pMarker->text );
          
          // Last frame
          pen.setColor( QColor(200, 200, 200) );
          Canvas.setPen(pen);
          Canvas.drawLine( points[3].x(), points[3].y(), points[0].x(), points[0].y() );
          Canvas.drawLine( points[0].x(), points[0].y(), points[1].x(), points[1].y() );
          pen.setColor( QColor(128, 128, 128) );
          Canvas.setPen(pen);
          Canvas.drawLine( points[3].x(), points[3].y(), points[2].x(), points[2].y() );
          Canvas.drawLine( points[2].x(), points[2].y(), points[1].x(), points[1].y() );

          txtHt += (txtSize.height()+1);

     }
}


//////////////////// END of Visualizing functions /////////////////////

bool CGraph :: SetData( QString lineName, double *buffer )
{
 CLine *pLine;
 int i;

     pLine = lineGetLineForName( lineName );

     if( !pLine )
     {
          u_msg("SetData: Линии с именем '%s' не сеществует", lineName.toAscii().data());
          return false;
     }

     memmove(pLine->data, buffer, pLine->size*sizeof(double));

     pLine->maxv = -INFINITY;
     pLine->minv =  INFINITY;

     for( i = 1;i < pLine->size;i++ )
     {
          // Find max value
          if( pLine->data[i] > pLine->maxv )
              pLine->maxv = pLine->data[i];
          // Find min value
          if( pLine->data[i] < pLine->minv )
              pLine->minv = pLine->data[i];
     }

     pLine->delta = pLine->maxv - pLine->minv;

     CalculateParams();

     for( i = 0;i < pView->nummarkers;i++ )
     {
    	 CMarker *pMarker = (CMarker *)pView->MarkerList.at(i);
         pMarker->Validate( 0, LineSize, minlinev, maxlinev );
     }

     NeedRefresh = true;

 return true;
}


double CGraph :: GetYvalue( QString lineName, double x )
{
 CLine *pLine;

     pLine = lineGetLineForName( lineName );

     if( !pLine )
     {
          u_msg("GetYvalue: No line with name '%s'", lineName.toAscii().data());
          return false;
     }

     if( x < 0 || x >= (double)LineSize )
         return 0;

 return pLine->data[ (int)x ];         
}


void CGraph :: CalculateParams()
{
 int i;
 CLine *pLine;

     if( LineList.count() == 0 )
     {
         maxlinedelta = maxlinev = minlinev = 0;
         return;
     }

     maxlinev = -INFINITY;
     minlinev = INFINITY;
     for( i = 0;i < LineList.count();i++ )
     {
          pLine = (CLine *)LineList.at(i);
          if( !pLine->visible )
               continue;
          if( pLine->maxv > maxlinev )
              maxlinev = pLine->maxv;
          if( pLine->minv < minlinev )
              minlinev = pLine->minv;
     }

     maxlinedelta = maxlinev - minlinev;

     ResetView();
}

bool CGraph :: BringToFront( QString lineName )
{
 int pos;

     if( (pos = lineGetNumForName( lineName )) == -1 )
     {
         u_msg("BringToFront: No line with name '%s'", lineName.toAscii().data());
         return false;
     }
     // Inverted order
     if( pos != LineList.count()-1 )
     {
         LineList.move( pos, LineList.count()-1 );
         NeedRefresh = true;
     }

 return true;
}


bool CGraph :: ClearLine( QString lineName )
{
 CLine *pLine;

     pLine = lineGetLineForName( lineName );

     if( !pLine )
     {
          u_msg("ClearLine: No line with name '%s'", lineName.toAscii().data());
          return false;
     }

     pLine->Clear();

     CalculateParams();

     NeedRefresh = true;

 return true;
}


bool CGraph :: ShowLine( QString lineName, bool Visible )
{
 CLine *pLine;

     pLine = lineGetLineForName( lineName );

     if( !pLine )
     {
          u_msg("ShowLine: No line with name '%s'", lineName.toAscii().data());
          return false;
     }

     if( pLine->Show( Visible ) )
         NeedRefresh = true;

     CalculateParams();

 return true;
}


bool CGraph :: ShowMarker( QString &markerName, bool Visible )
{
 CMarker *pMarker;

     pMarker = markerGetMarkerForName( markerName );

     if( !pMarker )
     {
          u_msg("ShowMarker: No marker with name '%s'", markerName.toAscii().data());
          return false;
     }

     if( pMarker->Show( Visible ) )
         NeedRefresh = true;

 return true;
}

void CGraph :: ShowAllMarkers( bool Visible )
{
 int i;
 CMarker *pMarker;

     for( i = 0;i < pView->nummarkers;i++ )
     {
          pMarker = (CMarker *)pView->MarkerList.at(i);
          if( pMarker->ismarker )
              if( pMarker->Show( Visible ) )
                  NeedRefresh = true;
     }
}

bool CGraph :: ActivateMarker( int ScrX, int ScrY )
{
 QString null_name = "";
 CMarker *pMarker;
 CVectInternal v, vl, vr;
 int i;
 bool found = false;

     // Clipping
	 v  = GetPosition( ScrX, ScrY );
     vl = GetPosition( ScrX-5, ScrY-5 );
     vr = GetPosition( ScrX+5, ScrY+5 );

     for( i = 0;i < pView->nummarkers;i++ )
     {
    	 pMarker = (CMarker *)pView->MarkerList.at(i);

    	 if( !pMarker->ismarker )
    		 continue;

    	 if( pMarker->visible && pMarker->position_x > vl.x && pMarker->position_x < vr.x )
    	 {
    		 found = true;
    		 break;
    	 }
     }

     if( pView->func )
    	 pView->func->onpressfunc( found ? pMarker->Name : null_name, v.x, v.y, ScrX, ScrY );

     for( i = 0;i < pView->nummarkers;i++ )
     {
          pMarker = (CMarker *)pView->MarkerList.at(i);

          if( !pMarker->ismarker )
               continue;

          if( pMarker->visible && pMarker->position_x > vl.x && pMarker->position_x < vr.x )
          {
              //u_msg("!");
        	  if( pMarker->style & MS_2D_VIEW )
        	  {
        		  if( pMarker->position_y < vl.y && pMarker->position_y > vr.y )
        		  {
        			  pMarker->active = true;
        			  return true;
        		  }
        	  }
        	  else
        	  {
        		  pMarker->active = true;
				  return true;
        	  }
          }
     }
 return false;
}

void CGraph :: DeActivateMarker()
{
 CMarker *pMarker;
 int i;

     for( i = 0;i < pView->nummarkers;i++ )
     {
          pMarker = (CMarker *)pView->MarkerList.at(i);
          if( pMarker->ismarker && pMarker->active )
          {
              pMarker->active = false;
              if( pMarker->func )
            	  pMarker->func->onreleasefunc( pMarker->Name, pMarker->position_x, pMarker->position_y );
          }
          //u_msg("!");
     }
}

bool CGraph :: MoveMarker( int ScrX, int ScrY )
{
 CMarker *pMarker;
 CVectInternal v;
 int i;


     // Clipping
     v = GetPosition( ScrX, ScrY );

     for( i = 0;i < pView->nummarkers;i++ )
     {
          pMarker = (CMarker *)pView->MarkerList.at(i);

          if( !pMarker->ismarker )
               continue;

          if( pMarker->active /*&& pMarker->visible*/ )
          {
              if( pMarker->style & MS_LOCKED )
              {
                  if( v.x < 0 )
                      v.x = 0;
                  if( v.x >= pView->LineSize )
                      v.x = pView->LineSize-1;
                  if( v.y < minlinev )
                	  v.y = minlinev;
                  if( v.y > maxlinev )
                      v.y = maxlinev;
              }
              if( pMarker->Set(v.x, v.y) )
                  NeedRefresh = true;
              return true;
          }
     }

 return false;     
}

bool CGraph :: GetMarkerPos( QString &markerName, double &x, double &y )
{
 CMarker *pMarker;

     pMarker = markerGetMarkerForName( markerName );

     if( !pMarker )
     {
          u_msg("GetMarkerPos: No marker with name '%s'", markerName.toAscii().data());
          return false;
     }

     pMarker->Get( x, y );

 return true;
}

bool CGraph :: SetMarkerPos( QString &markerName, double x, double y )
{
 CMarker *pMarker;

     pMarker = markerGetMarkerForName( markerName );

     if( !pMarker )
     {
          u_msg("SetMarkerPos: No marker with name '%s'", markerName.toAscii().data());
          return false;
     }

     if( pMarker->Set( x, y ) )
         NeedRefresh = true;

 return true;
}


bool CGraph :: SetMarkerText( QString &markerName, QString &markerText )
{
 CMarker *pMarker;

     pMarker = markerGetMarkerForName( markerName );

     if( !pMarker )
     {
          u_msg("SetMarkerText: No marker with name '%s'", markerName.toAscii().data());
          return false;
     }

     pMarker->text = markerText;

     NeedRefresh = true;

 return true;
}


bool CGraph :: GetMarkerAtPos( QString &markerName, double fromX, double toX, int &iterator, double &x, double &y, bool &IsVisible, bool &IsMarker )
{
 int i;
 CMarker *pMarker;

     if( iterator < 0 )
         iterator = 0;

     for( i = iterator;i < pView->nummarkers;i++ )
     {
          pMarker = (CMarker *)pView->MarkerList.at(i);

          if( pMarker->position_x >= fromX && pMarker->position_x < toX )
          {
              markerName = pMarker->Name;
              iterator   = ++i;
              x			 = pMarker->position_x;
              y			 = pMarker->position_y;
              IsVisible  = pMarker->visible;
              IsMarker   = pMarker->ismarker;
              return true;
          }
     }

     markerName = "";
     iterator   = i;
     x			= 0;
     y			= 0;
     IsVisible  = false;
     IsMarker   = false;

 return false;
}


bool CGraph :: GetMarkerByIdx( int idx, QString &markerName, double &x, double &y, bool &IsVisible, bool &IsMarker )
{
 CMarker *pMarker;


	if( idx < 0 || idx >= pView->nummarkers )
		return false;

	pMarker = (CMarker *)pView->MarkerList.at(idx);

	markerName = pMarker->Name;
	x		   = pMarker->position_x;
	y		   = pMarker->position_y;
	IsVisible  = pMarker->visible;
	IsMarker   = pMarker->ismarker;

 return true;
}


bool CGraph::IsMarkerVisible( QString &markerName, bool &vis )
{
 CMarker *pMarker;

	 pMarker = markerGetMarkerForName( markerName );

     if( !pMarker )
     {
          u_msg("IsMarkerVisible: No marker with name '%s'", markerName.toAscii().data());
          return false;
     }

     vis = pMarker->visible;

 return true;
}


bool CGraph :: MarkerExists( QString &markerName )
{
 return  markerGetNumForName( markerName ) == -1 ? false : true;
}


int  CGraph :: GetMarkerCount()
{
 return pView->nummarkers;
}


void CGraph :: ShowAllLabels( bool Visible )
{
 int i;
 CMarker *pMarker;

     for( i = 0;i < pView->nummarkers;i++ )
     {
          pMarker = (CMarker *)pView->MarkerList.at(i);
          if( !pMarker->ismarker )
              if( pMarker->Show( Visible ) )
                  NeedRefresh = true;
     }
}


