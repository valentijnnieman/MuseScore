/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "bowpressure.h"

#include "style/style.h"

#include "dummyparent.h"
#include "trill.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   BowPressureSegment
//---------------------------------------------------------

BowPressureSegment::BowPressureSegment(BowPressure* sp)
    : LineSegment(ElementType::BOW_PRESSURE_SEGMENT, sp, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
}

PropertyDelegate BowPressureSegment::resolvePropertyDelegate(Pid pid) const
{
    if (pid == Pid::BOW_PRESSURE_HEIGHT || pid == Pid::PLACEMENT) {
        return PropertyDelegate(spanner());
    }
    return LineSegment::resolvePropertyDelegate(pid);
}

//---------------------------------------------------------
//   BowPressure
//---------------------------------------------------------

static const ElementStyle bowPressureStyle {
    { Sid::bowPressurePlacement, Pid::PLACEMENT },
    { Sid::bowPressureHeight,    Pid::BOW_PRESSURE_HEIGHT },
};

static const ElementStyle bowPressureSegmentStyle {
    { Sid::bowPressureMinDistance, Pid::MIN_DISTANCE },
};

BowPressure::BowPressure(DummyParent* parent)
    : SLine(ElementType::BOW_PRESSURE, parent)
{
    initElementStyle(&bowPressureStyle);
}

LineSegment* BowPressure::createLineSegment()
{
    BowPressureSegment* seg = new BowPressureSegment(this);
    seg->setTrack(track());
    seg->setColor(lineColor());
    seg->initElementStyle(&bowPressureSegmentStyle);
    return seg;
}

PointF BowPressure::linePos(Grip grip, System** system) const
{
    return Trill::trillLinePos(this, grip, system);
}

Sid BowPressure::defaultPosSid() const
{
    return placeAbove() ? Sid::bowPressurePosAbove : Sid::bowPressurePosBelow;
}

PropertyValue BowPressure::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BOW_PRESSURE_HEIGHT:
        return m_height;
    default:
        break;
    }
    return SLine::getProperty(propertyId);
}

bool BowPressure::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::BOW_PRESSURE_HEIGHT:
        setHeight(val.value<Spatium>());
        break;
    case Pid::COLOR:
        setColor(val.value<Color>());
        setLineColor(val.value<Color>());
        break;
    default:
        return SLine::setProperty(propertyId, val);
    }
    triggerLayout();
    return true;
}

PropertyValue BowPressure::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BOW_PRESSURE_HEIGHT:
        return style().styleV(Sid::bowPressureHeight);
    case Pid::PLACEMENT:
        return style().styleV(Sid::bowPressurePlacement);
    default:
        return SLine::propertyDefault(propertyId);
    }
}
}
