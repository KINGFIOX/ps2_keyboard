// SPDX-License-Identifier: Unlicense
// SPDX-FileCopyrightText: 2026
package org.chipsalliance.ps2_keyboard.elaborator

import mainargs._
import org.chipsalliance.ps2_keyboard.PS2Keyboard
import circt.stage.ChiselStage

object PS2KeyboardMain {
  implicit object PathRead extends TokensReader.Simple[os.Path] {
    def shortName = "path"
    def read(strs: Seq[String]) = Right(os.Path(strs.head, os.pwd))
  }

  @main
  def design(
    @arg(name = "target-dir") targetDir: os.Path = os.pwd
  ) = {
    val fir = ChiselStage.emitCHIRRTL(new PS2Keyboard)
    os.write.over(targetDir / "PS2Keyboard.fir", fir)
  }

  def main(args: Array[String]): Unit = ParserForMethods(this).runOrExit(args.toIndexedSeq)
}
