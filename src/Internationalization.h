#ifndef STYLE_ANALYZER_INTERNATIONALIZATION_H
#define STYLE_ANALYZER_INTERNATIONALIZATION_H

#include <string>

namespace sa
{

using namespace std;

class InternationalizationEngine
{
public :
    string getTranslated (string pureString);

    static InternationalizationEngine& instance();

private :
    InternationalizationEngine();
};

}

#define saTranslate(str) sa::InternationalizationEngine::instance().getTranslated (str)

#endif // STYLE_ANALYZER_INTERNATIONALIZATION_H
