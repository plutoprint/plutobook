/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PLUTOBOOK_FRAGMENTBUILDER_H
#define PLUTOBOOK_FRAGMENTBUILDER_H

#include "box.h"

namespace plutobook {

enum class FragmentType {
    Column,
    Page
};

enum FragmentBoundaryRule { AssociateWithFormerFragment, AssociateWithLatterFragment };

class FragmentBuilder {
public:
    FragmentBuilder() = default;
    virtual ~FragmentBuilder() = default;

    virtual FragmentType fragmentType() const = 0;

    virtual float fragmentHeightForOffset(float offset) const = 0;
    virtual float fragmentRemainingHeightForOffset(float offset, FragmentBoundaryRule rule) const = 0;

    virtual void addForcedFragmentBreak(float offset) {}
    virtual void setFragmentBreak(float offset, float spaceShortage) {}
    virtual void updateMinimumFragmentHeight(float offset, float minHeight) {}

    float applyFragmentBreakBefore(const BoxFrame* child, float offset);
    float applyFragmentBreakAfter(const BoxFrame* child, float offset);
    float applyFragmentBreakInside(const BoxFrame* child, float offset);

    void enterFragment(float offset);
    void leaveFragment(float offset);

    float fragmentOffset() const;

    bool needsBreakBetween(BreakBetween between) const;
    bool needsBreakInside(BreakInside inside) const;

private:
    int64_t m_fragmentOffset = 0;
};

} // namespace plutobook

#endif // PLUTOBOOK_FRAGMENTBUILDER_H
