// SPDX-License-Identifier: Unlicense
// SPDX-FileCopyrightText: 2026

package org.chipsalliance.ps2_keyboard

import chisel3._
import chisel3.util._

/** Interface of [[PS2Keyboard]]. */
class PS2KeyboardInterface extends Bundle {
  val ps2Clk   = Input(Bool())
  val ps2Data  = Input(Bool())
  val nextdata = Input(Bool())
  val data     = Output(UInt(8.W))
  val valid    = Output(Bool())
  val overflow = Output(Bool())
}

/** Hardware implementation of PS/2 keyboard receiver. */
class PS2Keyboard
    extends FixedIOModule(new PS2KeyboardInterface)
    with ImplicitClock
    with ImplicitReset {

  // buffer, hold one byte of data
  // one byte consist of 11bit
  // [start, b0, b1, b2, b3, b4, b5, b6, b7, parity, stop]
  val buffer = Reg(Vec(10, Bool()))
  val count  = RegInit(0.U(4.W))

  // data queue
  val queue = Module( new Queue(UInt(8.W), entries = 8) )
  queue.io.enq.valid := false.B
  queue.io.enq.bits := Cat(buffer(8), buffer(7), buffer(6), buffer(5), buffer(4), buffer(3), buffer(2), buffer(1))
  queue.io.deq.ready := false.B

  // clock sync
  val ps2clk0Q = RegNext(io.ps2Clk)
  val ps2clk1Q = RegNext(ps2clk0Q)
  val ps2clk2Q = RegNext(ps2clk1Q)
  val samplingW = ps2clk2Q && !ps2clk1Q

  // io
  io.data     := queue.io.deq.bits
  io.valid    := queue.io.deq.valid
  // overflow
  // enq.valid -> !enq.ready
  io.overflow := false.B
  when(queue.io.enq.valid) {
    when(!queue.io.enq.ready) { // missing handshake
      io.overflow := true.B
    }
  }

  when(queue.io.deq.valid) {
    when(io.nextdata) {
      queue.io.deq.ready := true.B
    }
  }

  when(samplingW) {
    when(count === 10.U) {
      val startOk  = !buffer(0)
      val stopOk   = io.ps2Data
      // buffer[1:10) <=> buffer[1:9]
      val parityOk = buffer.slice(1, 10).reduce(_ ^ _)
      when(startOk && stopOk && parityOk) {
        queue.io.enq.valid := true.B
      }
      count := 0.U
    }.otherwise {
      buffer(count) := io.ps2Data
      count := count + 1.U
    }
  }

}
