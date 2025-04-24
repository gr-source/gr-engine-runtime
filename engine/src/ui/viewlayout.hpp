#pragma once

class ViewLayout {
public:
    virtual ~ViewLayout() {}

    virtual void onFocus() {}

    virtual void onBlur() {}

    virtual void onRender() {}
};









