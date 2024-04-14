/*
 * Color.h
 *
 * Created: 2020/02/12 0:03:41
 *  Author: masaki
 */
#ifndef COLOR_H_
#define COLOR_H_

struct ColorRGB;
struct ColorHLS;
struct Color2HLS;

struct ColorRGB
{
  uint8_t r; // RED   (0 - 255)
  uint8_t g; // GREEN (0 - 255)
  uint8_t b; // BLUE  (0 - 255)

  ColorRGB()
      : r(0), g(0), b(0)
  {
  }

  ColorRGB(uint8_t _r, uint8_t _g, uint8_t _b)
      : r(_r), g(_g), b(_b)
  {
  }

  ColorRGB(const ColorHLS &hls);
};

struct ColorHLS
{
  short h;   // HUE (0-360) 0:red 90:green 180:skyblue 270:purple
  uint8_t l; // LUM (0-255)
  uint8_t s; // SAT (0-255)

  ColorHLS()
      : h(0), l(0), s(255)
  {
  }

  ColorHLS(short hue, uint8_t lum, uint8_t sat = 255)
      : h(hue), l(lum), s(sat)
  {
  }

  ColorHLS(const ColorRGB &rgb);
};

struct Color2HLS
{
  ColorHLS foreColor;
  ColorHLS backColor;

  Color2HLS(ColorHLS color)
      : foreColor(color), backColor(0, 0, 0)
  {
  }

  Color2HLS(ColorHLS color1, ColorHLS color2)
      : foreColor(color1), backColor(color2)
  {
  }
};

static void
HLStoRGB(short h, uint8_t il, uint8_t is, uint8_t *rr, uint8_t *gg, uint8_t *bb)
{
  double l = il / 255.0;
  double s = is / 255.0;

  double r, g, b;
  r = 0;
  g = 0;
  b = 0;
  while (h < 0)
    h += 360;

  while (h > 360)
    h -= 360;

  if (s == 0)
  {
    *rr = 0;
    *gg = 0;
    *bb = 0;
    return;
  }

  double m2 = (l < 0.5) ? l * (1 + s) : l + s - l * s,
         m1 = l * 2 - m2,
         tmp;
  tmp = h + 120;
  if (tmp > 360)
    tmp = tmp - 360;

  if (tmp < 60)
    r = (m1 + (m2 - m1) * tmp / 60);
  else if (tmp < 180)
    r = m2;
  else if (tmp < 240)
    r = m1 + (m2 - m1) * (240 - tmp) / 60;
  else
    r = m1;

  tmp = h;
  if (tmp < 60)
    g = m1 + (m2 - m1) * tmp / 60;
  else if (tmp < 180)
    g = m2;
  else if (tmp < 240)
    g = m1 + (m2 - m1) * (240 - tmp) / 60;
  else
    g = m1;

  tmp = h - 120;
  if (tmp < 0)
    tmp = tmp + 360;

  if (tmp < 60)
    b = m1 + (m2 - m1) * tmp / 60;
  else if (tmp < 180)
    b = m2;
  else if (tmp < 240)
    b = m1 + (m2 - m1) * (240 - tmp) / 60;
  else
    b = m1;

  *rr = (uint8_t)(r * 255);
  *gg = (uint8_t)(g * 255);
  *bb = (uint8_t)(b * 255);
}

/*
static void
HLStoRGB (const HLS &hls, uint8_t *rr, uint8_t *gg, uint8_t *bb)
{
  HLStoRGB (hls.h, hls.l, hls.s, rr, gg, bb);
}
*/

// RGB -> HLS
static void
RGB2HLS(uint8_t ir, uint8_t ig, uint8_t ib, short *oh, uint8_t *ol, uint8_t *os)
{
  double r = (double)ir / 255.0;
  double g = (double)ig / 255.0;
  double b = (double)ib / 255.0;
  double maxval = max(r, max(g, b));
  double minval = min(r, min(g, b));

  double h, l, s;

  l = (maxval + minval) / 2.0;

  if (maxval == minval)
  {
    s = 0;
    h = 0;
  }
  else
  {
    if (l <= 0.5)
      s = (maxval - minval) / (maxval + minval);
    else
      s = (maxval - minval) / (2 - maxval - minval);

    /* Hue */
    double cr = (maxval - r) / (maxval - minval);
    double cg = (maxval - g) / (maxval - minval);
    double cb = (maxval - b) / (maxval - minval);

    if (r == maxval)
      h = cb - cg;
    else if (g == maxval)
      h = 2 + cr - cb;
    else
      h = 4 + cg - cr;

    h = 60.0 * h;
    if (h < 0)
      h = h + 360.0f;
  }

  *oh = (short)h;
  *ol = (uint8_t)(l * 255.0);
  *os = (uint8_t)(s * 255.0);
}

inline ColorRGB::ColorRGB(const ColorHLS &hls)
{
  HLStoRGB(hls.h, hls.l, hls.s, &r, &g, &b);
}

inline ColorHLS::ColorHLS(const ColorRGB &rgb)
{
  RGB2HLS(rgb.r, rgb.g, rgb.b, &h, &l, &s);
}

#endif /* COLORUTILITY_H_ */
