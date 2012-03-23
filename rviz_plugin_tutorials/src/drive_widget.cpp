/*
 * Copyright (c) 2011, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <math.h>

#include <QPainter>
#include <QMouseEvent>

#include "drive_widget.h"

namespace rviz_plugin_tutorials
{

DriveWidget::DriveWidget( QWidget* parent )
  : QWidget( parent )
  , linear_velocity_( 0 )
  , angular_velocity_( 0 )
  , linear_max_( 10 )
  , angular_max_( 2 )
{
}

void DriveWidget::paintEvent( QPaintEvent* event )
{
  QColor background;
  QColor crosshair;
  if( isEnabled() )
  {
    background = Qt::white;
    crosshair = Qt::black;
  }
  else
  {
    background = Qt::lightGray;
    crosshair = Qt::darkGray;
  }
  int w = width();
  int h = height();
  int size = (( w > h ) ? h : w) - 1;
  int hpad = ( w - size ) / 2;
  int vpad = ( h - size ) / 2;

  QPainter painter( this );
  painter.setBrush( background );
  painter.setPen( crosshair );
  painter.drawRect( QRect( hpad, vpad, size, size ));
  painter.drawLine( hpad, height() / 2, hpad + size, height() / 2 );
  painter.drawLine( width() / 2, vpad, width() / 2, vpad + size );

  if( isEnabled() && (angular_velocity_ != 0 || linear_velocity_ != 0 ))
  {
    QPen arrow;
    arrow.setWidth( size/20 );
    arrow.setColor( Qt::green );
    arrow.setCapStyle( Qt::RoundCap );
    arrow.setJoinStyle( Qt::RoundJoin );
    painter.setPen( arrow );

    int step_count = 100;
    QPointF left_track[ step_count ];
    QPointF right_track[ step_count ];

    float half_track_width = size/4.0;

    float cx = w/2;
    float cy = h/2;
    left_track[ 0 ].setX( cx - half_track_width );
    left_track[ 0 ].setY( cy );
    right_track[ 0 ].setX( cx + half_track_width );
    right_track[ 0 ].setY( cy );
    float angle = M_PI/2;
    float delta_angle = angular_velocity_ / step_count;
    float step_dist = linear_velocity_ * size/2 / linear_max_ / step_count;
    for( int step = 1; step < step_count; step++ )
    {
      angle += delta_angle / 2;
      float next_cx = cx + step_dist * cosf( angle );
      float next_cy = cy - step_dist * sinf( angle );
      angle += delta_angle / 2;

      left_track[ step ].setX( next_cx + half_track_width * cosf( angle + M_PI/2 ));
      left_track[ step ].setY( next_cy - half_track_width * sinf( angle + M_PI/2 ));
      right_track[ step ].setX( next_cx + half_track_width * cosf( angle - M_PI/2 ));
      right_track[ step ].setY( next_cy - half_track_width * sinf( angle - M_PI/2 ));

      cx = next_cx;
      cy = next_cy;
    }
    painter.drawPolyline( left_track, step_count );
    painter.drawPolyline( right_track, step_count );

    int left_arrow_dir = (-step_dist + half_track_width * delta_angle > 0);
    int right_arrow_dir = (-step_dist - half_track_width * delta_angle > 0);

    arrow.setJoinStyle( Qt::MiterJoin );
    painter.setPen( arrow );

    float head_len = size / 8.0;
    QPointF arrow_head[ 3 ];
    float x, y;
    if( fabsf( -step_dist + half_track_width * delta_angle ) > .01 )
    {
      x = left_track[ step_count - 1 ].x();
      y = left_track[ step_count - 1 ].y();
      arrow_head[ 0 ].setX( x + head_len * cosf( angle + 3*M_PI/4 + left_arrow_dir * M_PI ));
      arrow_head[ 0 ].setY( y - head_len * sinf( angle + 3*M_PI/4 + left_arrow_dir * M_PI ));
      arrow_head[ 1 ].setX( x );
      arrow_head[ 1 ].setY( y );
      arrow_head[ 2 ].setX( x + head_len * cosf( angle - 3*M_PI/4 + left_arrow_dir * M_PI ));
      arrow_head[ 2 ].setY( y - head_len * sinf( angle - 3*M_PI/4 + left_arrow_dir * M_PI ));
      painter.drawPolyline( arrow_head, 3 );
    }
    if( fabsf( -step_dist - half_track_width * delta_angle ) > .01 )
    {
      x = right_track[ step_count - 1 ].x();
      y = right_track[ step_count - 1 ].y();
      arrow_head[ 0 ].setX( x + head_len * cosf( angle + 3*M_PI/4 + right_arrow_dir * M_PI ));
      arrow_head[ 0 ].setY( y - head_len * sinf( angle + 3*M_PI/4 + right_arrow_dir * M_PI ));
      arrow_head[ 1 ].setX( x );
      arrow_head[ 1 ].setY( y );
      arrow_head[ 2 ].setX( x + head_len * cosf( angle - 3*M_PI/4 + right_arrow_dir * M_PI ));
      arrow_head[ 2 ].setY( y - head_len * sinf( angle - 3*M_PI/4 + right_arrow_dir * M_PI ));
      painter.drawPolyline( arrow_head, 3 );
    }
  }
}

void DriveWidget::mouseMoveEvent( QMouseEvent* event )
{
  sendVelocitiesFromMouse( event->x(), event->y(), width(), height() );
}

void DriveWidget::mousePressEvent( QMouseEvent* event )
{
  sendVelocitiesFromMouse( event->x(), event->y(), width(), height() );
}

void DriveWidget::leaveEvent( QEvent* event )
{
  stop();
}

void DriveWidget::sendVelocitiesFromMouse( int x, int y, int width, int height )
{  
  int size = (( width > height ) ? height : width );
  int hpad = ( width - size ) / 2;
  int vpad = ( height - size ) / 2;

  linear_velocity_ = (1.0 - float( y - vpad ) / float( size / 2 )) * linear_max_;
  angular_velocity_ = (1.0 - float( x - hpad ) / float( size / 2 )) * angular_max_;
  update();
  Q_EMIT outputVelocity( linear_velocity_, angular_velocity_ );
}

void DriveWidget::mouseReleaseEvent( QMouseEvent* event )
{
  stop();
}

void DriveWidget::stop()
{
  linear_velocity_ = 0;
  angular_velocity_ = 0;
  update();
  Q_EMIT outputVelocity( linear_velocity_, angular_velocity_ );
}

} // end namespace rviz_plugin_tutorials
