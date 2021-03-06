/*
 * Copyright 2001-2003 Sun Microsystems, Inc.  All Rights Reserved.
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

/**
 *  @test
 *  @bug 4406439 4925740
 *  @summary ClassesByName2 verifies that all the classes in the loaded class list can be found with classesByName..
 *
 *  @author Tim Bell (based on ClassesByName by Robert Field)
 *
 *  @run build TestScaffold VMConnection TargetListener TargetAdapter
 *  @run compile -g ClassesByName2Test.java
 *  @run main ClassesByName2Test
 */
import com.sun.jdi.*;
import com.sun.jdi.event.*;
import com.sun.jdi.request.*;

import java.util.*;

    /********** target program **********/

class ClassesByName2Targ {
    public static void ready() {
        System.out.println("Ready!");
    }

    public static void main(String[] args){
        System.out.println("Howdy!");
        try {

            Thread zero = new Thread ("ZERO") {
                    public void run () {
                        System.setProperty("java.awt.headless", "true");
                        java.awt.Toolkit tk = java.awt.Toolkit.getDefaultToolkit();

                    }
                };

            Thread one = new Thread ("ONE") {
                    public void run () {
                        try {
                            java.security.KeyPairGenerator keyGen =
                                java.security.KeyPairGenerator.getInstance("DSA", "SUN");
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                };

            Thread two = new Thread ("TWO") {
                    public void run () {
                        javax.rmi.CORBA.Util.getCodebase(this.getClass());
                    }
                };

            ready();

            two.start();
            one.start();
            zero.start();

            try {
                zero.join();
                one.join();
                two.join();
            } catch (InterruptedException iex) {
                iex.printStackTrace();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        System.out.println("Goodbye from ClassesByName2Targ!");
    }
}

    /********** test program **********/

public class ClassesByName2Test extends TestScaffold {

    ClassesByName2Test (String args[]) {
        super(args);
    }

    public static void main(String[] args)      throws Exception {
        new ClassesByName2Test(args).startTests();
    }

    protected void runTests() throws Exception {
        /*
         * Get to the top of ready()
         */
        startTo("ClassesByName2Targ", "ready", "()V");

        vm().resume();

        int i = 0;
        while (i < 8 && !vmDisconnected) {
            i++;
            List all = vm().allClasses();
            System.out.println("");
            System.out.println("++++ Lookup number: " + i + ".  allClasses() returned " +
                               all.size() + " classes.");
            for (Iterator it = all.iterator(); it.hasNext(); ) {
                ReferenceType cls = (ReferenceType)it.next();
                String name = cls.name();
                List found = vm().classesByName(name);
                if (found.contains(cls)) {
                    //System.out.println("Found class: " + name);
                } else {
                    System.out.println("CLASS NOT FOUND: " + name);
                    throw new Exception("CLASS NOT FOUND (by classesByName): " +
                                        name);
                }
            }
        }
        /*
         * resume the target listening for events
         */
        listenUntilVMDisconnect();

        /*
         * deal with results of test
         * if anything has called failure("foo") testFailed will be true
         */
        if (!testFailed) {
            println("ClassesByName2Test: passed");
        } else {
            throw new Exception("ClassesByName2Test: failed");
        }
    }
}
