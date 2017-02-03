/*****************************************************************************
* Copyright 2015-2017 Alexander Barthel albar965@mailbox.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef LITTLENAVMAP_MAPPAINTER_H
#define LITTLENAVMAP_MAPPAINTER_H

#include "common/coordinateconverter.h"
#include "common/maptypes.h"
#include "options/optiondata.h"

#include <marble/MarbleWidget.h>
#include <QPen>
#include <QApplication>

namespace atools {
namespace geo {
class Pos;
}
}

namespace Marble {
class GeoDataLineString;
class GeoPainter;
}

class SymbolPainter;
class MapLayer;
class MapQuery;
class MapScale;
class MapWidget;

/* Struct that is passed on each paint event to all painters */
struct PaintContext
{
  const MapLayer *mapLayer; /* layer for the current zoom distance also affected by detail level
                             *  should be used to visibility of map objects */
  const MapLayer *mapLayerEffective; /* layer for the current zoom distance not affected by detail level.
                                      *  Should be used to determine text visibility and object sizes. */
  Marble::GeoPainter *painter;
  Marble::ViewportParams *viewport;
  Marble::ViewContext viewContext;
  bool drawFast; /* true if reduced details should be used */
  maptypes::MapObjectTypes objectTypes; /* Object types that should be drawn */
  atools::geo::Rect viewportRect; /* Rectangle of current viewport */
  opts::MapScrollDetail mapScrollDetail; /* Option that indicates the detail level when drawFast is true */
  QFont defaultFont /* Default widget font */;

  opts::DisplayOptions dispOpts;

  float textSizeAircraftAi = 1.f;
  float symbolSizeNavaid = 1.f;
  float thicknessFlightplan = 1.f;
  float textSizeNavaid = 1.f;
  float symbolSizeAirport = 1.f;
  float symbolSizeAircraftAi = 1.f;
  float textSizeFlightplan = 1.f;
  float textSizeAircraftUser = 1.f;
  float symbolSizeAircraftUser = 1.f;
  float textSizeAirport = 1.f;
  float thicknessTrail = 1.f;
  float thicknessRangeDistance = 1.f;

  // Needs to be larger than number of highest level airports
  static Q_DECL_CONSTEXPR int MAX_OBJECT_COUNT = 2500;
  int objectCount = 0;
  bool objCount()
  {
    objectCount++;
    return objectCount > MAX_OBJECT_COUNT;
  }

  bool isOverflow()
  {
    return objectCount > MAX_OBJECT_COUNT;
  }

  bool  dOpt(const opts::DisplayOptions& opts) const
  {
    return dispOpts & opts;
  }

  /* Calculate real symbol size */
  int sz(float scale, int size) const
  {
    return static_cast<int>(std::round(scale * size));
  }

  int sz(float scale, float size) const
  {
    return static_cast<int>(std::round(scale * size));
  }

  int sz(float scale, double size) const
  {
    return static_cast<int>(std::round(scale * size));
  }

  float szF(float scale, int size) const
  {
    return scale * size;
  }

  float szF(float scale, float size) const
  {
    return scale * size;
  }

  float szF(float scale, double size) const
  {
    return scale * size;
  }

  /* Calculate and set font based on scale */
  void szFont(float scale) const;

};

/*
 * Base class for all map painters
 */
class MapPainter :
  public CoordinateConverter
{
  Q_DECLARE_TR_FUNCTIONS(MapPainter)

public:
  MapPainter(MapWidget *marbleWidget, MapQuery *mapQuery, MapScale *mapScale);
  virtual ~MapPainter();

  virtual void render(PaintContext *context) = 0;

protected:
  /* Set render hints for anti aliasing depending on the view context (still or animation) */
  void setRenderHints(Marble::GeoPainter *painter);

  /* Draw a circle and return text placement hints (xtext and ytext). Number of points used
   * for the circle depends on the zoom distance */
  void paintCircle(Marble::GeoPainter *painter, const atools::geo::Pos& centerPos,
                   int radiusNm, bool fast, int& xtext, int& ytext);

  /* Find text position along a great circle route
   *  @param x,y resulting text position
   *  @param pos1,pos2 start and end coordinates of the line
   *  @param bearing text bearing at the returned position
   */
  bool findTextPos(const atools::geo::Pos& pos1, const atools::geo::Pos& pos2, Marble::GeoPainter *painter,
                   int textWidth, int textHeight, int& x, int& y, float *bearing);

  /* Find text position along a great circle route
   *  @param x,y resulting text position
   *  @param pos1,pos2 start and end coordinates of the line
   *  @param bearing text bearing at the returned position
   *  @param distanceMeter distance between points
   */
  bool findTextPos(const atools::geo::Pos& pos1, const atools::geo::Pos& pos2, Marble::GeoPainter *painter,
                   float distanceMeter, int textWidth, int textHeight, int& x, int& y, float *bearing);

  /* Find text position along a rhumb line route
   *  @param x,y resulting text position
   *  @param pos1,pos2 start and end coordinates of the line
   *  @param distanceMeter distance between points
   */
  bool findTextPosRhumb(const atools::geo::Pos& pos1, const atools::geo::Pos& pos2,
                        Marble::GeoPainter *painter, float distanceMeter, int textWidth, int textHeight,
                        int& x, int& y);

  void drawLineString(const PaintContext *context, const Marble::GeoDataLineString& linestring);
  void drawLineString(const PaintContext *context, const atools::geo::LineString& linestring);

  void paintArc(QPainter *painter, float x1, float y1, float x2, float y2, float x0, float y0, bool left);
  void paintArc(QPainter *painter, const QPoint& p1, const QPoint& p2, const QPoint& p0, bool left);
  void paintArc(QPainter *painter, const QPointF& p1, const QPointF& p2, const QPointF& p0, bool left);

  void paintHold(QPainter *painter, float x, float y, float direction, float lengthNm, bool left);
  void paintProcedureTurn(QPainter *painter, float x, float y, float turnHeading, float distanceNm, bool left, QLineF* extensionLine);

  /* Evaluate 50 text placement positions along line */
  const float FIND_TEXT_POS_STEP = 0.02f;

  /* Minimum points to use for a circle */
  const int CIRCLE_MIN_POINTS = 16;
  /* Maximum points to use for a circle */
  const int CIRCLE_MAX_POINTS = 72;

  SymbolPainter *symbolPainter;
  MapWidget *mapWidget;
  MapQuery *query;
  MapScale *scale;

};

#endif // LITTLENAVMAP_MAPPAINTER_H
