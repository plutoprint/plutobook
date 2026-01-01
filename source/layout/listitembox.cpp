/*
 * Copyright (c) 2022-2026 Samuel Ugochukwu <sammycageagle@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "listitembox.h"

namespace plutobook {

ListItemBox::ListItemBox(Node* node, const RefPtr<BoxStyle>& style)
    : BlockFlowBox(node, style)
{
}

InsideListMarkerBox::InsideListMarkerBox(const RefPtr<BoxStyle>& style)
    : InlineBox(nullptr, style)
{
}

OutsideListMarkerBox::OutsideListMarkerBox(const RefPtr<BoxStyle>& style)
    : BlockFlowBox(nullptr, style)
{
}

} // namespace plutobook
