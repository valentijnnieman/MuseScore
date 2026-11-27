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

#include <gtest/gtest.h>

#include "engraving/dom/bowpressure.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/linkedobjects.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/staff.h"

#include "engraving/rendering/score/tlayout.h"

#include "utils/scorerw.h"

using namespace mu::engraving;
using mu::engraving::rendering::score::TLayout;

static const String BOWPRESSURE_DATA_DIR("bowpressure_data/");

namespace {
// The triangle's vertical extent at the left and right edge of a slice
// (bowPressureTriangle() puts the base corners first, then the right edge, any apex, and the left edge)
double rightEdgeThickness(const PolygonF& poly)
{
    return std::abs(poly.at(2).y() - poly.at(0).y());
}

double leftEdgeThickness(const PolygonF& poly)
{
    return std::abs(poly.back().y() - poly.at(0).y());
}

BowPressure* bowPressureAt(Score* score, const Fraction& tick)
{
    for (const auto& [startTick, spanner] : score->spannerMap().map()) {
        if (spanner->isBowPressure() && spanner->tick() == tick) {
            return toBowPressure(spanner);
        }
    }
    return nullptr;
}

BowPressure* createBowPressure(Score* score, const Fraction& tick, const Fraction& ticks)
{
    BowPressure* bowPressure = Factory::createBowPressure(score->dummy());
    bowPressure->setTrack(0);
    bowPressure->setTrack2(0);
    bowPressure->setTick(tick);
    bowPressure->setTicks(ticks);
    return bowPressure;
}
}

class Engraving_BowPressureTests : public ::testing::Test
{
};

//---------------------------------------------------------
//   Two slices of one triangle: split at 2/3 of its duration,
//   and split exactly at the apex
//---------------------------------------------------------

TEST_F(Engraving_BowPressureTests, slicedTriangle)
{
    const double h = 3.0;

    // the first slice starts at a point and contains the apex (4 corners + apex)
    PolygonF first = TLayout::bowPressureTriangle(8.0, h, 0.0, 2.0 / 3.0, true);
    EXPECT_DOUBLE_EQ(leftEdgeThickness(first), 0.0);
    EXPECT_EQ(first.size(), 5u);

    // the second slice continues at the same thickness, ends at a point and has no apex
    PolygonF second = TLayout::bowPressureTriangle(4.0, h, 2.0 / 3.0, 1.0, true);
    EXPECT_DOUBLE_EQ(leftEdgeThickness(second), rightEdgeThickness(first));
    EXPECT_DOUBLE_EQ(rightEdgeThickness(second), 0.0);
    EXPECT_EQ(second.size(), 4u);

    // split at the apex: neither slice gets an apex point, both reach the full height at the split
    PolygonF rising = TLayout::bowPressureTriangle(6.0, h, 0.0, 0.5, true);
    PolygonF falling = TLayout::bowPressureTriangle(6.0, h, 0.5, 1.0, true);
    EXPECT_EQ(rising.size(), 4u);
    EXPECT_EQ(falling.size(), 4u);
    EXPECT_DOUBLE_EQ(rightEdgeThickness(rising), h);
    EXPECT_DOUBLE_EQ(leftEdgeThickness(falling), h);
}

//---------------------------------------------------------
//   A mark across a system break is laid out as two slices
//   of one triangle that line up at the break
//---------------------------------------------------------

TEST_F(Engraving_BowPressureTests, layoutAcrossSystemBreak)
{
    MasterScore* score = ScoreRW::readScore(BOWPRESSURE_DATA_DIR + u"bowpressure-systems.mscx");
    ASSERT_TRUE(score);

    // bars 1-3, line break after bar 2
    BowPressure* bowPressure = bowPressureAt(score, Fraction(0, 1));
    ASSERT_TRUE(bowPressure);
    ASSERT_EQ(bowPressure->spannerSegments().size(), 2u);

    BowPressureSegment* first = toBowPressureSegment(bowPressure->frontSegment());
    BowPressureSegment* second = toBowPressureSegment(bowPressure->backSegment());
    EXPECT_EQ(first->spannerSegmentType(), SpannerSegmentType::BEGIN);
    EXPECT_EQ(second->spannerSegmentType(), SpannerSegmentType::END);
    EXPECT_NE(first->system(), second->system());

    const PolygonF& firstTriangle = first->ldata()->triangle;
    const PolygonF& secondTriangle = second->ldata()->triangle;
    const double apexHeight = bowPressure->height().val() * first->spatium();

    // the break is at 2/3 of the duration, so the thickness there is 2/3 of the apex height on both sides
    EXPECT_DOUBLE_EQ(rightEdgeThickness(firstTriangle), apexHeight * 2.0 / 3.0);
    EXPECT_DOUBLE_EQ(leftEdgeThickness(secondTriangle), rightEdgeThickness(firstTriangle));

    // only the first slice reaches the apex
    EXPECT_DOUBLE_EQ(firstTriangle.boundingRect().height(), apexHeight);
    EXPECT_LT(secondTriangle.boundingRect().height(), apexHeight);

    delete score;
}

//---------------------------------------------------------
//   Deleting the end of a mark's range shortens it instead of removing it
//---------------------------------------------------------

TEST_F(Engraving_BowPressureTests, rangeDeleteShortens)
{
    MasterScore* score = ScoreRW::readScore(BOWPRESSURE_DATA_DIR + u"bowpressure-notes.mscx");
    ASSERT_TRUE(score);

    // bars 1-2
    BowPressure* bowPressure = createBowPressure(score, Fraction(0, 1), Fraction(2, 1));
    score->addSpanner(bowPressure);
    score->doLayout();

    // delete the contents of bar 2
    Measure* secondMeasure = score->firstMeasure()->nextMeasure();
    score->startCmd(TranslatableString::untranslatable("Delete range"));
    score->select(secondMeasure, SelectType::RANGE, 0);
    score->cmdDeleteSelection();
    score->endCmd();

    ASSERT_EQ(bowPressureAt(score, Fraction(0, 1)), bowPressure);
    EXPECT_EQ(bowPressure->ticks(), Fraction(1, 1));

    delete score;
}

//---------------------------------------------------------
//   Adding a mark also adds a linked copy to a linked staff
//---------------------------------------------------------

TEST_F(Engraving_BowPressureTests, addedToLinkedStaff)
{
    MasterScore* score = ScoreRW::readScore(BOWPRESSURE_DATA_DIR + u"bowpressure-notes.mscx");
    ASSERT_TRUE(score);

    Staff* staff = score->staff(0);
    Staff* linkedStaff = Factory::createStaff(staff->part());
    linkedStaff->initFromStaffType(staff->staffType(Fraction(0, 1)));
    score->undoInsertStaff(linkedStaff, 1, false);
    Excerpt::cloneStaff(staff, linkedStaff);
    score->doLayout();

    score->startCmd(TranslatableString::untranslatable("Add bow pressure"));
    score->undoAddElement(createBowPressure(score, Fraction(0, 1), Fraction(1, 1)));
    score->endCmd();

    std::vector<BowPressure*> bowPressures;
    for (const auto& [tick, spanner] : score->spannerMap().map()) {
        if (spanner->isBowPressure()) {
            bowPressures.push_back(toBowPressure(spanner));
        }
    }
    ASSERT_EQ(bowPressures.size(), 2u);

    std::set<staff_idx_t> staves { bowPressures[0]->staffIdx(), bowPressures[1]->staffIdx() };
    EXPECT_EQ(staves, std::set<staff_idx_t>({ 0, 1 }));

    ASSERT_TRUE(bowPressures[0]->links());
    EXPECT_TRUE(bowPressures[0]->links()->contains(bowPressures[1]));

    delete score;
}
