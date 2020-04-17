/* tab:4
 *
 * text.h - font data and text to mode X conversion utility header file
 *
 * "Copyright (c) 2004-2009 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Steve Lumetta
 * Version:       2
 * Creation Date: Thu Sep 9 22:08:16 2004
 * Filename:      text.h
 * History:
 *    SL    1    Thu Sep 9 22:08:16 2004
 *        First written.
 *    SL    2    Sat Sep 12 13:40:11 2009
 *        Integrated original release back into main code base.
 */

#ifndef TEXT_H
#define TEXT_H

/* The default VGA text mode font is 8x16 pixels. */
#define FONT_WIDTH   8
#define FONT_HEIGHT  16
#define SCREEN_WIDTH  320 /* the screen width */
#define SCROLL_WIDTH  (SCREEN_WIDTH / 4) /* the scroll width */
#define STATUS_BAR_SCREEN_SIZE   5760 /* (FONT_HEIGHT + 2) * IMAGE_X_DIM = 18 * 320 = 5760 */
#define STATUS_BAR_SCROLL_SIZE   (STATUS_BAR_SCREEN_SIZE / 4)   /* the space needed for one plane of a status bar */

/* Standard VGA text font. */
extern unsigned char font_data[256][16];

/* A buffer that holds a graphical image of the ASCII characters in the string */
extern unsigned char text_buffer[STATUS_BAR_SCREEN_SIZE];

/* A text to graphics image generation routine */
extern void write_text_to_graph(const char* str, int position);

#endif /* TEXT_H */
