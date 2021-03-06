/*
 * Copyright 2000-2005 Sun Microsystems, Inc.  All Rights Reserved.
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
 *  
 */
// This file is a derivative work resulting from (and including) modifications
// made by Azul Systems, Inc.  The date of such changes is 2010.
// Copyright 2010 Azul Systems, Inc.  All Rights Reserved.
//
// Please contact Azul Systems, Inc., 1600 Plymouth Street, Mountain View, 
// CA 94043 USA, or visit www.azulsystems.com if you need additional information 
// or have any questions.
#ifndef METHODCOMPARATOR_HPP
#define METHODCOMPARATOR_HPP


#include "bytecodes.hpp"
#include "growableArray.hpp"
#include "refsHierarchy_pd.hpp"
class BciMap;
class BytecodeStream;

// methodComparator provides an interface for determining if methods of
// different versions of classes are equivalent or switchable

class MethodComparator {
 friend void methodComparator_static_init();

 private:
  static BytecodeStream *_s_old, *_s_new;
  static constantPoolRef _old_cp, _new_cp;
  static BciMap *_bci_map;
  static bool _switchable_test;
  static GrowableArray<int> *_fwd_jmps;

  static bool args_same(Bytecodes::Code c_old, Bytecodes::Code c_new);
  static int check_stack_and_locals_size(methodOop old_method, methodOop new_method);

 public:
  static constantPoolOop old_cp() { return lvb_constantPoolRef(&_old_cp).as_constantPoolOop(); }
  static constantPoolOop new_cp() { return lvb_constantPoolRef(&_new_cp).as_constantPoolOop(); }

  // Check if the new method is equivalent to the old one modulo constant pool (EMCP).
  // Intuitive definition: two versions of the same method are EMCP, if they don't differ
  // on the source code level. Practically, we check whether the only difference between
  // method versions is some constantpool indices embedded into the bytecodes, and whether
  // these indices eventually point to the same constants for both method versions.
  static bool methods_EMCP(methodOop old_method, methodOop new_method);

  static bool methods_switchable(methodOop old_method, methodOop new_method, BciMap &bci_map);
};


// ByteCode Index Map. For two versions of the same method, where the new version may contain
// fragments not found in the old version, provides a mapping from an index of a bytecode in
// the old method to the index of the same bytecode in the new method.

class BciMap {
 private:
  int *_old_bci, *_new_st_bci, *_new_end_bci;
  int _cur_size, _cur_pos;
  int _pos;

 public:
  BciMap() {
    _cur_size = 50;
    _old_bci = (int*) malloc(sizeof(int) * _cur_size);
    _new_st_bci = (int*) malloc(sizeof(int) * _cur_size);
    _new_end_bci = (int*) malloc(sizeof(int) * _cur_size);
    _cur_pos = 0;
  }

  ~BciMap() {
    free(_old_bci);
    free(_new_st_bci);
    free(_new_end_bci);
  }

  // Store the position of an added fragment, e.g.
  //
  //                              |<- old_bci
  // -----------------------------------------
  // Old method   |invokevirtual 5|aload 1|...
  // -----------------------------------------
  //                                                         
  //                                 |<- new_st_bci          |<- new_end_bci    
  // --------------------------------------------------------------------
  // New method       |invokevirual 5|aload 2|invokevirtual 6|aload 1|...
  // --------------------------------------------------------------------
  //                                 ^^^^^^^^^^^^^^^^^^^^^^^^ 
  //                                    Added fragment 

  void store_fragment_location(int old_bci, int new_st_bci, int new_end_bci) {
    if (_cur_pos == _cur_size) {
      _cur_size += 10;
      _old_bci = (int*) realloc(_old_bci, sizeof(int) * _cur_size);
      _new_st_bci = (int*) realloc(_new_st_bci, sizeof(int) * _cur_size);
      _new_end_bci = (int*) realloc(_new_end_bci, sizeof(int) * _cur_size);
    }
    _old_bci[_cur_pos] = old_bci;
    _new_st_bci[_cur_pos] = new_st_bci;
    _new_end_bci[_cur_pos] = new_end_bci;
    _cur_pos++;
  }

  int new_bci_for_old(int old_bci) {
    if (_cur_pos == 0 || old_bci < _old_bci[0]) return old_bci;
    _pos = 1;
    while (_pos < _cur_pos && old_bci >= _old_bci[_pos])
      _pos++;
    return _new_end_bci[_pos-1] + (old_bci - _old_bci[_pos-1]);
  }

  // Test if two indexes - one in the old method and another in the new one - correspond
  // to the same bytecode
  bool old_and_new_locations_same(int old_dest_bci, int new_dest_bci) {
    if (new_bci_for_old(old_dest_bci) == new_dest_bci)
      return true;
    else if (_old_bci[_pos-1] == old_dest_bci)
      return (new_dest_bci == _new_st_bci[_pos-1]);
    else return false;
  }
};

#endif // METHODCOMPARATOR_HPP
