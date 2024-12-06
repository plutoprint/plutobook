#ifndef PLUTOBOOK_PAGEBUILDER_H
#define PLUTOBOOK_PAGEBUILDER_H

namespace plutobook {

class Book;

class PageBuilder {
public:
    explicit PageBuilder(const Book* book);

public:
    const Book* m_book;
};

} // namespace plutobook

#endif // PLUTOBOOK_PAGEBUILDER_H
