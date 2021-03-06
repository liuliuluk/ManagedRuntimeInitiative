/*
 * Copyright 2005 Sun Microsystems, Inc.  All Rights Reserved.
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

/*
 * @test
 * @bug 6238815
 * @summary test the new interface Addressable
 * @author Shanliang JIANG
 * @run clean AddressableTest
 * @run build AddressableTest
 * @run main AddressableTest
 */

import java.util.*;
import java.net.MalformedURLException;
import java.io.IOException;

import javax.management.*;
import javax.management.remote.*;
import javax.management.remote.rmi.*;

public class AddressableTest {
    private static final String[] protocols = {"rmi", "iiop"};
    private static final String[] prefixes = {"stub", "ior"};

    private static final MBeanServer mbs = MBeanServerFactory.createMBeanServer();

    public static void main(String[] args) throws Exception {
        System.out.println(">>> test the new interface Addressable.");
        boolean ok = true;

        for (int i = 0; i < protocols.length; i++) {
            try {
                test(protocols[i], prefixes[i]);

                System.out.println(">>> Test successed for "+protocols[i]);
            } catch (Exception e) {
                System.out.println(">>> Test failed for "+protocols[i]);
                e.printStackTrace(System.out);
                ok = false;
            }
        }

        if (ok) {
            System.out.println(">>> All Test passed.");
        } else {
            System.out.println(">>> Some TESTs FAILED");
            System.exit(1);
        }
    }

    public static void test(String proto, String prefix) throws Exception {
        JMXServiceURL url = new JMXServiceURL("service:jmx:" + proto + "://");
        JMXConnectorServer server =
                    JMXConnectorServerFactory.newJMXConnectorServer(url, null, mbs);

        server.start();

        JMXServiceURL serverAddr1 = server.getAddress();
        System.out.println(">>> Created a server with address "+serverAddr1);

        JMXConnector client1 = JMXConnectorFactory.connect(serverAddr1);
        JMXServiceURL clientAddr1 = ((JMXAddressable)client1).getAddress();

        System.out.println(">>> Created a client with address "+clientAddr1);

        if (!serverAddr1.equals(clientAddr1)) {
            throw new RuntimeException("The "+proto+" client does not return correct address.");
        }

        int i = clientAddr1.toString().indexOf(prefix);

        JMXServiceURL clientAddr2 =
            new JMXServiceURL("service:jmx:"+proto+":///"+clientAddr1.toString().substring(i));

        JMXConnector client2 = JMXConnectorFactory.connect(clientAddr2);

        System.out.println(">>> Created a client with address "+clientAddr2);

        if (!clientAddr2.equals(((JMXAddressable)client2).getAddress())) {
            throw new RuntimeException("The "+proto+" client does not return correct address.");
        }

        System.out.println(">>> The new client's host is "+clientAddr2.getHost()
                               +", port is "+clientAddr2.getPort());

        client1.close();
        client2.close();
        server.stop();
    }
}
