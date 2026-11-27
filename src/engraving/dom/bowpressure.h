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

#pragma once

#include "line.h"

namespace mu::engraving {
class BowPressure;

//---------------------------------------------------------
//   @@ BowPressureSegment
//    One system's worth of a bow pressure triangle.
//---------------------------------------------------------

class BowPressureSegment final : public LineSegment
{
    OBJECT_ALLOCATOR(engraving, BowPressureSegment)
    DECLARE_CLASSOF(ElementType::BOW_PRESSURE_SEGMENT)

public:
    BowPressureSegment(BowPressure* sp);

    BowPressureSegment* clone() const override { return new BowPressureSegment(*this); }

    BowPressure* bowPressure() const { return toBowPressure(spanner()); }

    PropertyDelegate resolvePropertyDelegate(Pid) const override;
};

//---------------------------------------------------------
//   @@ BowPressure
//    Filled isosceles triangle telling string players to
//    press the bow into the string; the apex height
//    indicates the amount of pressure.
//   @P bowPressureHeight  spatium  height of the apex
//---------------------------------------------------------

class BowPressure final : public SLine
{
    OBJECT_ALLOCATOR(engraving, BowPressure)
    DECLARE_CLASSOF(ElementType::BOW_PRESSURE)

public:
    BowPressure(DummyParent* parent);

    BowPressure* clone() const override { return new BowPressure(*this); }
    void setOwnershipParent(EngravingObject*) = delete;

    Anchor anchor() const override { return Anchor::SEGMENT; }

    LineSegment* createLineSegment() override;
    PointF linePos(Grip grip, System** system) const override;

    Spatium height() const { return m_height; }
    void setHeight(Spatium val) { m_height = val; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

protected:
    Sid defaultPosSid() const override;

private:
    Spatium m_height;
};
} // namespace mu::engraving
