# Backlight

*Jack-of-all-trades LED strip controller*

This is the first version of my LED strip controller. This code ran from mid 2015 to early 2017 with minimal modification on an Arduino hooked up to a strip of 12V RGB LEDs behind my desk. It reliably provided a nice set of colours to instantly theme my otherwise colour-neutral workspace, and crossfade functionality made it more pleasant than the default controller supplied with the strip.

In April 2017, I replaced backlight with [desker][] to reduce bloat and introduce compatiability with SK6812 individually-addressable strips.

Here's the breadboard layout:

![Breadboard layout][breadboard]

### Supported Modes

1. Solid colour
1. Strobe (selectable frequencies)
1. Music synchronisation via MSGEQ7
1. Colour detection via TCS3200

### Features

1. Control with IR remote
1. Colour crossfade in solid, strobe and colour detect Modes1
1. Status output to HD44780 16x2 character LCD display

## Useful Links

- Have a look at the [video][yt] I made demonstrating the MSGEQ7 music synchronisation.
- Check out my [blog post][site] on the same subject.

[yt]: https://youtu.be/vmSqzH9hZJs
[site]: https://albertnis.com/posts/afterglow/
[breadboard]: http://i.imgur.com/pIRoXSz.jpg
[desker]: https://github.com/albertnis/desker