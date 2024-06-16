#include "columnbuilder.h"
#include "multicolumnbox.h"
#include "linelayout.h"

namespace plutobook {

ColumnBuilder::ColumnBuilder(ColumnBuilder* parent, MultiColumnFlowBox* column)
    : m_parent(parent), m_column(column)
{
}

void ColumnBuilder::columnize()
{
    if(m_column->isChildrenInline()) {
        m_column->lineLayout()->columnize(*this, 0);
    } else {
        auto child = m_column->firstBoxFrame();
        while(child) {
            if(!child->isFloatingOrPositioned())
                child->columnize(*this, 0);
            child = child->nextBoxFrame();
        }
    }
}

} // namespace plutobook
