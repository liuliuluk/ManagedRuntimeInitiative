/*
 * Copyright 2004 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Sun designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Sun in the LICENSE file that accompanied this code.
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

package sun.jvmstat.perfdata.monitor;

import sun.jvmstat.monitor.*;
import sun.management.counter.Variability;
import java.nio.ByteBuffer;

/**
 * Class for monitoring a constant PerfData String instrument. The
 * value associated with a constant string instrument is fixed at
 * the string instrument's creation time. Its value and length never
 * change.
 *
 * @author Brian Doherty
 * @since 1.5
 */
public class PerfStringConstantMonitor extends PerfStringMonitor {

    /**
     * The cached value of the string instrument.
     */
    String data;

    /**
     * Constructor to create a StringMonitor object for the constant string
     * instrument object represented by the data in the given buffer.
     *
     * @param name the name of the instrumentation object
     * @param supported support level indicator
     * @param bb the buffer containing the string instrument data
     */
    public PerfStringConstantMonitor(String name, boolean supported,
                                     ByteBuffer bb) {
        super(name, Variability.CONSTANT, supported, bb);
        this.data = super.stringValue();
    }

    /**
     * {@inheritDoc}
     */
    public Object getValue() {
        return data;        // return the cached string
    }

    /**
     * {@inheritDoc}
     */
    public String stringValue() {
        return data;        // return the cached string
    }
}
