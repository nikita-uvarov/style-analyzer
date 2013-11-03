#include "Internationalization.h"

using namespace sa;

string sa::InternationalizationEngine::getTranslated (string pureString)
{
    return pureString;
}

InternationalizationEngine& sa::InternationalizationEngine::instance()
{
    static InternationalizationEngine theInstance;
    return theInstance;
}

sa::InternationalizationEngine::InternationalizationEngine()
{}
