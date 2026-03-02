// SPDX-License-Identifier: Unlicense
// SPDX-FileCopyrightText: 2026

package org.chipsalliance.ps2_keyboard

import chisel3._
import chisel3.experimental.hierarchy.{instantiable, Instance, Instantiate}
import chisel3.properties.{AnyClassType, Class, Property}
import chisel3.util.Cat

/** Interface of [[PS2Keyboard]]. */
class PS2KeyboardInterface extends Bundle {
  val clock    = Input(Clock())
  val reset    = Input(Bool())
  val ps2Clk   = Input(Bool())
  val ps2Data  = Input(Bool())
  val nextdata = Input(Bool())
  val data     = Output(UInt(8.W))
  val ready    = Output(Bool())
  val overflow = Output(Bool())
}

/** Hardware implementation of PS/2 keyboard receiver. */
@instantiable
class PS2Keyboard
    extends FixedIORawModule(new PS2KeyboardInterface)
    with ImplicitClock
    with ImplicitReset {
  override protected def implicitClock: Clock = io.clock
  override protected def implicitReset: Reset = io.reset

  // Internal signals, aligned with the original Verilog implementation.
  val buffer = Reg(Vec(10, Bool()))
  val fifo   = Reg(Vec(8, UInt(8.W)))
  val wPtr   = RegInit(0.U(3.W))
  val rPtr   = RegInit(0.U(3.W))
  val count  = RegInit(0.U(4.W))

  val readyReg    = RegInit(false.B)
  val overflowReg = RegInit(false.B)

  val ps2ClkSync = RegInit(0.U(3.W))
  ps2ClkSync := Cat(ps2ClkSync(1, 0), io.ps2Clk)
  val sampling = ps2ClkSync(2) && !ps2ClkSync(1)

  when(readyReg) {
    when(io.nextdata) {
      rPtr := rPtr + 1.U
      when(wPtr === (rPtr + 1.U)) {
        readyReg := false.B
      }
    }
  }

  when(sampling) {
    when(count === 10.U) {
      val startOk  = !buffer(0)
      val stopOk   = io.ps2Data
      val parityOk = buffer.slice(1, 10).reduce(_ ^ _)
      when(startOk && stopOk && parityOk) {
        fifo(wPtr) := Cat(buffer(8), buffer(7), buffer(6), buffer(5), buffer(4), buffer(3), buffer(2), buffer(1))
        wPtr := wPtr + 1.U
        readyReg := true.B
        overflowReg := overflowReg || (rPtr === (wPtr + 1.U))
      }
      count := 0.U
    }.otherwise {
      buffer(count) := io.ps2Data
      count := count + 1.U
    }
  }

  io.data     := fifo(rPtr)
  io.ready    := readyReg
  io.overflow := overflowReg
}
