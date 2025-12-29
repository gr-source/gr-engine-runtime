#pragma once

class ViewLayout
{
public:
    virtual ~ViewLayout() {}

    virtual const char* getName() const { return "Unknow"; }

    virtual void onFocus() {}

    virtual void onBlur() {}

    virtual void onRender() {}

    virtual void drop(void *data) {}
};









