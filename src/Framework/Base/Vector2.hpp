#pragma once
#include<utility>
#include<cmath>

class Vector2 final
{
    public:
    Vector2()=default;
    Vector2(double x,double y):x(x),y(y){}
    Vector2(const Vector2& copy):x(copy.x),y(copy.y){}
    Vector2(Vector2&& mov):x(std::move(mov.x)),y(std::move(mov.y)){}
    ~Vector2()=default;

    Vector2 operator=(const Vector2& copy)
    {
        x=copy.x;
        y=copy.y;
        return *this;
    }

    Vector2& operator=(Vector2&& mov)
    {
        x=std::move(mov.x);
        y=std::move(mov.y);
        return *this;
    }

    Vector2& operator+(const Vector2& rVector2)
    {
        x+=rVector2.x;
        y+=rVector2.y;
        return *this;
    }

    Vector2& operator-(const Vector2& rVector2)
    {
        x-=rVector2.x;
        y-=rVector2.y;
        return *this;
    }

    double operator*(const Vector2& rVector2)
    {
        return x*rVector2.x+y*rVector2.y;
    }

    double module()
    {
        return std::sqrt(x*x+y*y);
    }

    private:
    float x,y;
};

