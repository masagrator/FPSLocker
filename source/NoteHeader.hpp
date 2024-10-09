#include <tesla.hpp>

namespace tsl {
    namespace elm {

        u32 NoteHeaderDefaultHeight = 40;

        class NoteHeader : public Element {
        public:
            NoteHeader(const std::string &title, bool hasSeparator = false, tsl::Color headerBarColor = {0xF, 0xF, 0xF, 0xF}) : m_text(title), m_hasSeparator(hasSeparator), m_headerBarColor(headerBarColor) {}
            virtual ~NoteHeader() {}

            virtual void draw(gfx::Renderer *renderer) override {
                renderer->drawRect(this->getX() - 2, this->getBottomBound() - 30, 3, 23, this->m_headerBarColor);
                renderer->drawString(this->m_text.c_str(), false, this->getX() + 13, this->getBottomBound() - 12, 15, a(tsl::style::color::ColorText));

                if (this->m_hasSeparator)
                    renderer->drawRect(this->getX(), this->getBottomBound(), this->getWidth(), 1, a(tsl::style::color::ColorFrame));
            }

            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                // Check if the NoteHeader is part of a list and if it's the first entry in it, half it's height
                if (List *list = dynamic_cast<List*>(this->getParent()); list != nullptr) {
                    if (list->getIndexInList(this) == 0) {
                        this->setBoundaries(this->getX(), this->getY(), this->getWidth(), NoteHeaderDefaultHeight / 2);
                        return;
                    }
                }

                this->setBoundaries(this->getX(), this->getY(), this->getWidth(), NoteHeaderDefaultHeight);
            }

            virtual bool onClick(u64 keys) {
                return false;
            }

            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
                return nullptr;
            }

            inline void setText(const std::string &text) {
                this->m_text = text;
            }

            inline const std::string& getText() const {
                return this->m_text;
            }

        private:
            std::string m_text;
            bool m_hasSeparator;
            tsl::Color m_headerBarColor;
        };
    }
}