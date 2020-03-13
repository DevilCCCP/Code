#pragma once

inline int Square(int x, int y)
{
  return (x - y) * (x - y);
}

inline int Square(int r)
{
  return (r) * (r);
}

struct Point
{
  int X;
  int Y;

  bool operator == (const Point& other)
  { return IsEq(*this, other); }

  bool operator != (const Point& other)
  { return !IsEq(*this, other); }

  bool IsEmpty()
  { return IsEq(*this, Empty()); }

  static bool IsEq(const Point& a, const Point& b)
  { return a.X == b.X && a.Y == b.Y; }

  static const Point& Empty()
  {
    static Point emptyPoint;
    return emptyPoint;
  }

  Point(int x, int y)
    : X(x), Y(y)
  { }

  Point()
    : X(0x7fffffff), Y(0x7fffffff)
  { }
};

struct Rect
{
  int Left;
  int Top;
  int Width;
  int Height;

  bool operator == (const Rect& other)
  { return IsEq(*this, other); }

  bool operator != (const Rect& other)
  { return !IsEq(*this, other); }

  bool IsEmpty()
  { return IsEq(*this, Empty()); }

  static bool IsEq(const Rect& a, const Rect& b)
  { return a.Left == b.Left && a.Top == b.Top && a.Width == b.Width && a.Height == b.Height; }

  static const Rect& Empty()
  {
    static Rect emptyRect;
    return emptyRect;
  }

  Rect(int l, int t, int w, int h)
    : Left(l), Top(t), Width(w), Height(h)
  { }

  Rect()
    : Left(0), Top(0), Width(-1), Height(-1)
  { }
};

struct Rectangle
{
  int Left;
  int Top;
  int Right;
  int Bottom;

  bool operator == (const Rectangle& other)
  { return IsEq(*this, other); }

  bool operator != (const Rectangle& other)
  { return !IsEq(*this, other); }

  bool IsEmpty()
  { return IsEq(*this, Empty()); }

  static bool IsEq(const Rectangle& a, const Rectangle& b)
  { return a.Left == b.Left && a.Top == b.Top && a.Right == b.Right && a.Bottom == b.Bottom; }

  static const Rectangle& Empty()
  {
    static Rectangle emptyRectangle;
    return emptyRectangle;
  }

  int Width() const
  {
    return Right - Left + 1;
  }

  int Height() const
  {
    return Bottom - Top + 1;
  }

  Rectangle(int l, int t, int r, int b)
    : Left(l), Top(t), Right(r), Bottom(b)
  { }

  Rectangle()
    : Left(0), Top(0), Right(-1), Bottom(-1)
  { }
};
