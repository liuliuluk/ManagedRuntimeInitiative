/*
 * Copyright (c) 2007 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 */

import java.awt.*;
import java.util.Properties;
import sun.awt.*;

public class StringWidth extends Frame {

  public StringWidth() {
    Font plain = new Font("Dialog", Font.PLAIN, 10);
    Font bold = new Font("Dialog", Font.BOLD, 10);
    Properties props = new Properties();
    int x, y;

    // we must have visible Frame context for PrintDialog in Solaris
    setSize(400, 300);
    setVisible(true);

    PrintJob pj = getToolkit().getPrintJob(this, "", props);
    if (pj == null) {
        return;
    }
    Graphics  pg = pj.getGraphics();


    String test = "Hello World!";

    FontMetrics plainFm = pg.getFontMetrics(plain);
    FontMetrics boldFm = pg.getFontMetrics(bold);
    Dimension size = pj.getPageDimension();

    // now right justify on the printed page
    int center = size.width/2;
    y = 150;
    x = center - plainFm.stringWidth(test);
    pg.setFont(plain);
    pg.drawString(test, x-1, y);

    pg.dispose();
    pj.end();
    setVisible(false);
    System.exit(0);
  }

  public static void main(String[] args) {
    new StringWidth();
  }

}