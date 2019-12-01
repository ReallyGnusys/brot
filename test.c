#include <gtk/gtk.h>
#include <stdlib.h>
#include "brot.h"

#define IMAGE_WIDTH DIM
#define IMAGE_HEIGHT    DIM
//guchar rgbbuf[IMAGE_WIDTH * IMAGE_HEIGHT * 4];

typedef struct {
  gint x1;
  gint y1;
  gint width;
  gint height;
} SelectBox;

static int currentframe,renderedframes,viewframe;
static cairo_surface_t *images[MAXFRAMES];
static guchar  *rgbbuffers[MAXFRAMES];

static SelectBox *selectbox[MAXFRAMES];

typedef struct{
              double x;
              double y;
              double incr;
              double range;
              } FParams;  // frame parameters

static FParams *fparams[MAXFRAMES];


cairo_t *cr;
GdkPixbuf *pixbuf;

static GtkWidget *statuslab;


gboolean on_darea_expose (GtkWidget *widget,
              GdkEventExpose *event,
              gpointer user_data);

gboolean on_darea_motion (GtkWidget *widget,
              GdkEventMotion *event,
              gpointer user_data);

gboolean on_darea_clicked (GtkWidget *widget,
              GdkEventButton *event,
              gpointer user_data);
gboolean on_darea_unclicked (GtkWidget *widget,
              GdkEventButton *event,
              gpointer user_data);

const  cairo_rectangle_int_t  crect = { 0,0,DIM,DIM};

static gint
backbutton_press_event (GtkWidget *widget, GdkEventButton *event);

static gint
fwdbutton_press_event (GtkWidget *widget, GdkEventButton *event);

static gint
savebutton_press_event (GtkWidget *widget, GdkEventButton *event);

/* This callback quits the program */
static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    gtk_main_quit ();
    return FALSE;
}


void statusstring(double x, double y, double zoom,  int frame, char *buff){

  sprintf(buff,"X=%10.3e   Y=%10.3e : ZOOM=%9.2e :  FRAME %2d ",x,y,zoom,frame);


}


#define EVENT_METHOD(i, x) GTK_WIDGET_GET_CLASS(i)->x

static double gx1;
static double gy1;
static double grange;
static double incr;
static double zoom;
static double startrange;


static char _statusstr[64];

static void updatestatuslabel(){
  statusstring(gx1,gy1,zoom,currentframe,_statusstr);

  gtk_label_set_text((GtkLabel *)statuslab, _statusstr);

}

void DbgFParams(const char *str){
  int i;
  printf("\nDbgFParams : %s",str);
  for (i = 0 ; i < 10 ; i++)
    printf("\nfparams[%d] = %p ",i,fparams[i]);
  printf("\n");

}

//GtkWidget *darea;
int
main (int argc, char *argv[])
{
  GtkWidget *window, *darea, *table,*hrule,*vrule;
  GtkWidget *box1,*hbox,*hbox2,*backbutton,*fwdbutton,*savebutton,*separator;

  gint x, y;
  guchar *pos;

  currentframe = 0;
  renderedframes = 0;
  viewframe = 0;
  for ( x = 0 ; x < MAXFRAMES ; x++){
    rgbbuffers[x] = (guchar *)NULL;
    images[x] = (cairo_surface_t *)NULL;
    selectbox[x] = (SelectBox *)NULL;
    fparams[x] = (FParams *)NULL;
  }

  gtk_init (&argc, &argv);
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

 /* Set the window title */
  gtk_window_set_title (GTK_WINDOW (window), "Brot");


  statusstring(0.0,0.0,1.0,1,_statusstr);


  // box1 = vbox
  box1 = gtk_vbox_new (FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 5);

  backbutton = gtk_button_new_with_label ("<<");
  gtk_box_pack_start (GTK_BOX (hbox), backbutton, FALSE, FALSE, 5);
  gtk_widget_show (backbutton);


  fwdbutton = gtk_button_new_with_label (">>");
  gtk_box_pack_start (GTK_BOX (hbox), fwdbutton, FALSE, FALSE, 5);
  gtk_widget_show (fwdbutton);

  separator = gtk_vseparator_new ();
  gtk_box_pack_start (GTK_BOX (hbox), separator, FALSE, TRUE, 5);
  gtk_widget_show (separator);

  savebutton = gtk_button_new_with_label ("Save");
  gtk_box_pack_start (GTK_BOX (hbox), savebutton, FALSE, FALSE, 5);
  gtk_widget_show (savebutton);






  /*
  separator = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (box1), separator, FALSE, TRUE, 5);
  gtk_widget_show (separator);

  hbox2 = gtk_hbox_new (FALSE, 5);
  */


  /* Create a table for placing the ruler and the drawing area */
  //table = gtk_table_new (3, 2, FALSE);

  //gtk_container_add (GTK_CONTAINER (window), table);
  /* Set a handler for delete_event that immediately
     * exits GTK. */
  g_signal_connect (window, "delete-event",
                      G_CALLBACK (delete_event), NULL);



  darea = gtk_drawing_area_new ();


  gtk_widget_set_size_request (darea, IMAGE_WIDTH, IMAGE_HEIGHT);

  /*
  gtk_box_pack_start (GTK_BOX (hbox2), darea, FALSE, FALSE, 0);

  gtk_widget_show (hbox2);
  */
  gtk_box_pack_start (GTK_BOX (box1), darea, FALSE, FALSE, 0);

  statuslab = gtk_label_new( _statusstr);

  gtk_box_pack_start (GTK_BOX (box1), statuslab, FALSE, FALSE, 0);


  gtk_box_pack_start (GTK_BOX (box1), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);


  // gtk_box_pack_start (GTK_BOX (box1), hbox2, FALSE, FALSE, 0);

  gtk_container_add (GTK_CONTAINER (window), box1);




  //gtk_table_attach (GTK_TABLE (table), darea, 1, 2, 1, 2,
  //                    GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 0);

  gtk_widget_set_events( darea,GDK_POINTER_MOTION_MASK |
                         GDK_POINTER_MOTION_HINT_MASK |GDK_BUTTON_PRESS_MASK |
                         GDK_EXPOSURE_MASK | GDK_BUTTON_RELEASE_MASK );


  gtk_widget_set_events( backbutton,GDK_BUTTON_PRESS_MASK);
  gtk_widget_set_events( fwdbutton,GDK_BUTTON_PRESS_MASK);

  gtk_widget_set_events( savebutton,GDK_BUTTON_PRESS_MASK);






  gx1 = -2.5;
  gy1 = -2.0;
  grange = 4;

  startrange = grange;
  incr = grange/DIM;
  zoom = 1.0;

  fparams[currentframe] = malloc(sizeof(FParams));
  fparams[currentframe]->x = gx1;
  fparams[currentframe]->y = gy1;
  fparams[currentframe]->range = grange;
  fparams[currentframe]->incr = incr;


  updatestatuslabel();
  /*
  statusstring(x,y,incr,currentframe,_statusstr);

  gtk_label_set_text((GtkLabel *)statuslab, _statusstr);
  */

  g_signal_connect_swapped (darea, "motion_notify_event",
                            GTK_SIGNAL_FUNC (on_darea_motion),
                            statuslab);
                            //                              G_CALLBACK (EVENT_METHOD (button1, motion_notify_event)),

  gtk_signal_connect (GTK_OBJECT (darea), "expose-event",
                      GTK_SIGNAL_FUNC (on_darea_expose), NULL);



   gtk_signal_connect (GTK_OBJECT (darea), "button_press_event",
        GTK_SIGNAL_FUNC (on_darea_clicked),
        NULL);
   gtk_signal_connect (GTK_OBJECT (darea), "button_release_event",
        GTK_SIGNAL_FUNC (on_darea_unclicked),
        NULL);
   /*
   gtk_signal_connect (GTK_OBJECT (backbutton), "button_press_event",
        GTK_SIGNAL_FUNC (backbutton_press_event),
        NULL);
   */

   g_signal_connect_swapped (backbutton, "button_press_event",
        GTK_SIGNAL_FUNC (backbutton_press_event),
                               darea);

   g_signal_connect_swapped (fwdbutton, "button_press_event",
        GTK_SIGNAL_FUNC (fwdbutton_press_event),
                               darea);

   g_signal_connect_swapped (savebutton, "button_press_event",
        GTK_SIGNAL_FUNC (savebutton_press_event),
                               darea);

   /*
   gtk_signal_connect (GTK_OBJECT (savebutton), "button_press_event",
        GTK_SIGNAL_FUNC (savebutton_press_event),
        NULL);
   */

   gtk_widget_set_app_paintable(window, TRUE);
   gtk_widget_show_all (window);



  /* Set up the RGB buffer. */
   guchar *rgbbuf;


   rgbbuf = (guchar *)  brot_draw(DIM, gx1 , gy1, incr );
   if(rgbbuf == NULL){

     printf("brot_draw failed\n");
     exit(-1);
   }

   printf("\nbrot_draw ok - stride is %d\n",cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,DIM));
  rgbbuffers[currentframe] = rgbbuf;

   images[currentframe] = cairo_image_surface_create_for_data(rgbbuf,
                                              CAIRO_FORMAT_ARGB32,DIM,DIM,
                                              cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,DIM));

   renderedframes++;

  gtk_main ();



  for(x = 0 ; x < renderedframes ; x++)
    cairo_surface_destroy(images[x]);


  return 0;
}


static void PrintSelBox(SelectBox *box){
  printf("\nSelectBox x1 %d y1 %d width %d height %d\n",box->x1,box->y1,box->width,box->height);

}





gboolean
on_darea_expose (GtkWidget *widget,
         GdkEventExpose *event,
         gpointer user_data)
{


  /*
  gdk_draw_rgb_32_image (widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
              0, 0, IMAGE_WIDTH, IMAGE_HEIGHT,
              GDK_RGB_DITHER_MAX, rgbbuf, IMAGE_WIDTH * 4);
  return TRUE;
  */
  int renderframe = viewframe/2;
  char fname[128];


  cairo_t *cr;
  cairo_surface_t *target,*outp;


  sprintf(fname,"%s_F%d.png","BROTOUT",viewframe);

  cr = gdk_cairo_create(widget->window);

  cairo_set_source_surface(cr, images[renderframe], 0, 0);

  // CAIRO_OPERATOR_SOURCE prevents blending of alpha keyed buffer data
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

  cairo_paint(cr);

 if (viewframe%2){

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.25);

    cairo_rectangle (cr, selectbox[renderframe]->x1,selectbox[renderframe]->y1,
                     selectbox[renderframe]->width,selectbox[renderframe]->height);
    cairo_fill (cr);


  }


 if ((user_data) &&  (*((int *)user_data) == 333)){
    printf("\ndarea EXPOSE EVENT - user-data set to 333\n");
    target = cairo_get_target (cr);


    printf("\n width %d height %d\n",cairo_image_surface_get_width(target),cairo_image_surface_get_height(target));

    target = cairo_surface_map_to_image(target,&crect);

    cairo_surface_write_to_png(target,fname);
    //cairo_surface_write_to_png(images[renderframe],fname);
  }

  cairo_destroy(cr);

  /*
  printf("\non_darea_expose\n");

  cairo_paint(cr);

  cairo_fill(cr);
  */
  return FALSE;


}



gboolean
saveviewframetopng (int frame, char *basename)

{


  /*
  gdk_draw_rgb_32_image (widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
              0, 0, IMAGE_WIDTH, IMAGE_HEIGHT,
              GDK_RGB_DITHER_MAX, rgbbuf, IMAGE_WIDTH * 4);
  return TRUE;
  */

  char fname[128];
  if (frame > (renderedframes * 2 -1))
    return FALSE;

  int renderframe = frame/2;



  cairo_t *cr;
  cairo_surface_t *image;


  sprintf(fname,"%s_F%d.png",basename,frame);
  image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,DIM,DIM);


    /*
    cairo_image_surface_create_for_data(rgbbuffers[renderframe],
                                              CAIRO_FORMAT_ARGB32,DIM,DIM,
                                        cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,DIM));

    */
  cr = cairo_create(image);


  cairo_set_source_surface(cr, images[renderframe], 0, 0);

  // CAIRO_OPERATOR_SOURCE prevents blending of alpha keyed buffer data
   cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

  cairo_paint(cr);

 if (frame%2){

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.25);

    cairo_rectangle (cr, selectbox[renderframe]->x1,selectbox[renderframe]->y1,
                     selectbox[renderframe]->width,selectbox[renderframe]->height);
    cairo_fill (cr);


  }

 cairo_surface_write_to_png(image,fname);
 cairo_destroy(cr);

  /*
  printf("\non_darea_expose\n");

  cairo_paint(cr);

  cairo_fill(cr);
  */
  return FALSE;


}

double pixtocoordX(gint x){
  return gx1 + grange * (double)x/(double)DIM;
}

double pixtocoordY(gint y){
  return gy1 + grange * (double)y/(double)DIM;
}




gboolean
on_darea_motion (GtkWidget *widget,
         GdkEventMotion *event,
         gpointer user_data)
{

  // this should be called only for statuslab - as signal swapped
  double x,y;
  x = pixtocoordX(event->x);
  y = pixtocoordY(event->y);
  //printf("\non_darea_motion ");
  //printf("\n x %f  y %f  state %X\n",x,y,event->state);

  statusstring(x,y,zoom,currentframe,_statusstr);
  gtk_label_set_text((GtkLabel *)widget, _statusstr);

 return TRUE;
}



static gint
backbutton_press_event (GtkWidget *widget, GdkEventButton *event)
{

  //printf("\n backbutton press event  : button = %d\n",event->button);
  printf("\n backbutton press event  : viewframe %d currentframe %d \n",viewframe,currentframe);
  //if (event->button == 1 && pixmap != NULL)
  //   draw_brush (widget, event->x, event->y);

  // DbgFParams("backbutton_press_event");

 if(viewframe > 0){
   viewframe--;

   printf("\n backbutton dec viewframe  : viewframe %d currentframe %d \n",viewframe,currentframe);


   if(currentframe != (viewframe/2)){
    currentframe = (viewframe/2);
    printf("\n backbutton update currentframe  : viewframe %d currentframe %d \n",viewframe,currentframe);

    // reset frame params

    if(fparams[currentframe] == NULL) {
      printf("\n error : fparams[currentframe] == NULL\n");
    }

    gx1 = fparams[currentframe]->x;
    gy1 =fparams[currentframe]->y;
    grange = fparams[currentframe]->range;
    incr = fparams[currentframe]->incr;

    zoom = startrange/grange;

    updatestatuslabel();

   }
   on_darea_expose (widget,
                     (GdkEventExpose *)NULL,
                     NULL);
 }


  return TRUE;
}


static gint
fwdbutton_press_event (GtkWidget *widget, GdkEventButton *event)
{

  //printf("\n backbutton press event  : button = %d\n",event->button);
  printf("\n fwdbutton press event  : currentframe %d renderedframes %d \n",currentframe,renderedframes);
  //if (event->button == 1 && pixmap != NULL)
  //   draw_brush (widget, event->x, event->y);
  if(viewframe < (renderedframes*2 -2) ){
   viewframe++;



   if(currentframe != (viewframe/2)){
    currentframe = (viewframe/2);

    //if((currentframe+1) < renderedframes){
    //currentframe++;

    // reset frame params

    gx1 = fparams[currentframe]->x;
    gy1 =fparams[currentframe]->y;
    grange = fparams[currentframe]->range;
    incr = fparams[currentframe]->incr;
    zoom = startrange/grange;

    updatestatuslabel();
   }
   on_darea_expose (widget,
                    (GdkEventExpose *)NULL,
                    NULL);
}


  return TRUE;
}



static gint
savebutton_press_event (GtkWidget *widget, GdkEventButton *event)
{

  int tmp333 = 333;

  int frame,renderframe;
  printf("\n savebutton press event  : ENTRY button = %d\n",event->button);

  //int viewframe;
  viewframe = 0;

  while (1){
    renderframe = viewframe/2;
    if (renderframe > currentframe) // save up to current frame
      break;

    if (images[renderframe] == NULL)
      break;

    if ((viewframe%2) && (selectbox[renderframe] == NULL))
      break;

    printf("Saving frame %d\n",viewframe);
    on_darea_expose (widget,
                     (GdkEventExpose *)NULL,
                     (gpointer)&tmp333);
    viewframe++;

  }

  viewframe--;

  /*
  for(frame = 0 ; frame < renderedframes*2 - 1; frame++){

    if(selectbox[frame] == NULL)
      break;
    saveviewframetopng(frame,"BROT");
  }
  printf("\n savebutton press event  : EXIT button = %d\n",event->button);
  */

  //if (event->button == 1 && pixmap != NULL)
  //   draw_brush (widget, event->x, event->y);

  return TRUE;
}




double sx1,sx2,sy1,sy2;
int px1,px2,py1,py2;

gboolean
on_darea_clicked (GtkWidget *widget,
         GdkEventButton *event,
         gpointer user_data)
{
  int x, y;
  double xf,yf;
  GdkModifierType state;
  x = event->x;
  y = event->y;
  state = event->state;

  xf =  pixtocoordX(x); //gx1 + grange * (float)x/(float)DIM;
  //yf =  gy1 + grange * (float)(DIM - y - 1)/(float)DIM;
  yf =  pixtocoordY(y); // gy1 + grange * (float)y/(float)DIM;


  printf("\n darea clicked!");
  printf("\n x %d y %d state %d",x,y,state);
  printf("\n xf %.3f yf %.3f state\n",xf,yf);

  sx1 = xf;
  sy1 = yf;

  px1 = x;
  py1 = y;

  return TRUE;
}


gboolean
on_darea_unclicked (GtkWidget *widget,
         GdkEventButton *event,
         gpointer user_data)
{
  int x, y;
  double xf,yf,tf;
  GdkModifierType state;
  x = event->x;
  y = event->y;
  state = event->state;

  xf =  gx1 + grange * (double)x/(double)DIM;
  //yf =  gy1 + grange * (float)(DIM - y - 1)/(float)DIM;
  yf =  gy1 + grange * (double)y/(double)DIM;


  printf("\n darea UNclicked!");
  printf("\n x %d y %d state %d",x,y,state);
  printf("\n xf %.3f yf %.3f state\n",xf,yf);

  sx2 = xf;
  sy2 = yf;

  if ( sx2 < sx1) {
    tf = sx2;
    sx2 = sx1;
    sx1 = tf;
  }

  if ( sy2 < sy1) {
    tf = sy2;
    sy2 = sy1;
    sy1 = tf;
  }


  // now we would like to check that if important coords (e.g. 0 are in picture, they are exactly represented numerically)
  // we accept the computed incr without change, but tweak the x1,y1 if necessary

  // printf("\nsx1 %f sy1 %f",sx1,sy1);
  //printf("\nsx2 %f sy2 %f\n",sx2,sy2);

  double newrange,newincr, fpixrange;
  int pixrange;
  if (( sx2 - sx1) > ( sy2 - sy1))
    newrange = sx2 - sx1;
  else
    newrange = sy2 - sy1;

  fpixrange = ((double)DIM) * newrange/grange;

  //printf("\nNEW range %f  OLD range %f   fpixrange %f\n",newrange,grange,fpixrange);

  pixrange = (int)fpixrange;
  //printf("\nNEW range %f  OLD range %f   fpixrange %f pixrange %d \n",newrange,grange,fpixrange,pixrange);

  newincr = newrange/(double)DIM;





  gx1 = sx1;
  gy1 = sy1;
  grange = newrange;
  incr = newincr;

  int tmp;
  px2 =x;
  py2 = y;

  if (px2 < px1) {
    tmp = px1;
    px1 = px2;
    px2 = px1 + pixrange;
  } else{

    px2 = px1 + pixrange;
  }



  if (py2 < py1) {
    tmp = py1;
    py1 = py2;
    py2 = py1 + pixrange;
  } else{

    py2 = py1 + pixrange;
  }


  // gdk_draw_rectangle(pixmap,widget->style->white_gc,
  //                   FALSE,px1,py1,px1+pixrange,
  //                   py1+pixrange);

  /*
  cairo_t *cr;

  cr = gdk_cairo_create(widget->window);
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_rectangle (cr, 0, 0, 100.0, 100.0);
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_fill (cr);


  cairo_destroy(cr);

  */

  if (selectbox[currentframe] == NULL)
    selectbox[currentframe] = malloc(sizeof(SelectBox));

  selectbox[currentframe]->x1 = px1;
  selectbox[currentframe]->y1 = py1;
  selectbox[currentframe]->width = pixrange;
  selectbox[currentframe]->height = pixrange;

  PrintSelBox(selectbox[currentframe]);

#if 1

  cairo_t *cr;

  //cr = gdk_cairo_create(widget->window);

  cr = gdk_cairo_create(widget->window);

  // CAIRO_OPERATOR_SOURCE prevents blending of alpha keyed buffer data



  if (selectbox[currentframe]){

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.25);
    cairo_rectangle (cr, selectbox[currentframe]->x1,selectbox[currentframe]->y1,
                     selectbox[currentframe]->width,selectbox[currentframe]->height);
    cairo_fill (cr);


  }
  cairo_destroy(cr);


#endif


  viewframe++;
  on_darea_expose (widget,
                   (GdkEventExpose *)NULL,
                   NULL);




  currentframe++;
  viewframe =currentframe *2;
  // store new frame params

  if(fparams[currentframe] == NULL)
    fparams[currentframe] = malloc(sizeof(FParams));
  fparams[currentframe]->x = gx1;
  fparams[currentframe]->y = gy1;
  fparams[currentframe]->range = grange;
  fparams[currentframe]->incr = incr;

  zoom = startrange/grange;


  printf("\nnew zoom = %f  startrange %f grange %f \n",zoom,startrange,grange);
  printf("sizeof(guchar) %ld",sizeof(guchar));
  guchar *rgbbuf;


   rgbbuf = (guchar *)  brot_draw(DIM, gx1 , gy1, incr );
   if(rgbbuf == NULL){

     printf("brot_draw failed\n");
     exit(-1);
   }

   if(rgbbuffers[currentframe]!= NULL)
     free(rgbbuffers[currentframe]);

   rgbbuffers[currentframe] = rgbbuf;

   if(images[currentframe] != NULL)
      cairo_surface_destroy(images[currentframe]);

   images[currentframe] = cairo_image_surface_create_for_data(rgbbuf,
                                                              CAIRO_FORMAT_ARGB32,DIM,DIM,
                                                              cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,DIM));



//destroy any allocated images or params or selectboxes  above this

   // DbgFParams("L21");
   // should be no selectbox for this frame now
   if (selectbox[currentframe] != NULL){
     free(selectbox[currentframe]);
     selectbox[currentframe] = NULL;
   }
   int tmp1;


   for (tmp1 = currentframe+1;tmp1 < renderedframes ; tmp1++){
     if (selectbox[tmp1] != NULL){
       free(selectbox[tmp1]);
       selectbox[tmp1] = NULL;
     }
     if (fparams[tmp1] != NULL){
       free(fparams[tmp1]);
       fparams[tmp1] = NULL;
     }
     if (images[tmp1] != NULL){
       cairo_surface_destroy(images[tmp1]);
       images[tmp1] = NULL;
     }

     if(rgbbuffers[tmp1] != NULL){
       free(rgbbuffers[tmp1]);
       rgbbuffers[tmp1] = NULL;
     }


   }

   // DbgFParams("L22");

  // rendered frames is always one more than currentframe immediately after drawing
   renderedframes = currentframe + 1;

   updatestatuslabel();

   on_darea_expose (widget,
                   (GdkEventExpose *)NULL,
                   user_data);

  return TRUE;
}
