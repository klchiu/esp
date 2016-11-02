------------------------------------------------------------------------------
--  This file is part of an extension to the GRLIB VHDL IP library.
--  Copyright (C) 2013, System Level Design (SLD) group @ Columbia University
--
--  GRLIP is a Copyright (C) 2008 - 2013, Aeroflex Gaisler
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  To receive a copy of the GNU General Public License, write to the Free
--  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
--  02111-1307  USA.
-----------------------------------------------------------------------------
-- Package: 	acctypes
-- File:	acctypes.vhd
-- Author:	Paolo Mantovani - SLD @ Columbia University
-- Description:	SLD accelerators common types
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package acctypes is

  constant MAXREGNUM : integer := 32;
  type bank_type is array (natural range <>) of std_logic_vector(31 downto 0);

  -- bank(0): CMD (reset if cleared)
  constant CMD_REG : integer range 0 to MAXREGNUM - 1:= 0;
  constant CMD_BIT_START : integer range 0 to 31 := 0;
  constant CMD_BIT_LAST  : integer range 0 to 31 := 0;

  -- bank(1): STATUS (idle when cleared) - Read only
  constant STATUS_REG : integer range 0 to MAXREGNUM - 1 := 1;
  constant STATUS_BIT_RUN  : integer range 0 to 31 := 0;
  constant STATUS_BIT_DONE : integer range 0 to 31 := 1;
  constant STATUS_BIT_ERR  : integer range 0 to 31 := 2;
  constant STATUS_BIT_LAST : integer range 0 to 31 := 2;
  
  -- bank(2)        : SELECT (which accelerator will run in 1 hot encoding)
  constant SELECT_REG : integer range 0 to MAXREGNUM - 1 := 2;
  
  -- bank(3)        : RESERVED - Read only
  constant DEVID_REG : integer range 0 to MAXREGNUM - 1 := 3;

  -- bank(4)        : PT_ADDRESS (page table bus address)
  constant PT_ADDRESS_REG : integer range 0 to MAXREGNUM - 1 := 4;
  
  -- bank(5)        : PT_NCHUNK (number of physical contiguous buffers in memory)
  constant PT_NCHUNK_REG : integer range 0 to MAXREGNUM - 1 := 5;
  
  -- bank(6)        : PT_SHIFT (log2(cunk size))
  constant PT_SHIFT_REG : integer range 0 to MAXREGNUM - 1 := 6;
  
  -- bank(7)        : PT_NCHUNK_MAX (maximum number of chunks supported) - Read only
  constant PT_NCHUNK_MAX_REG : integer range 0 to MAXREGNUM - 1 := 7;
  
  -- bank(8)        : RESERVED

  -- bank(9)        : RESERVED

  -- bank(10)       : RESERVED

  -- bank(11)       : RESERVED

  -- bank(12)       : SRC_OFFSET (offset in bytes from beginning of physical buffer)
  constant SRC_OFFSET_REG : integer range 0 to MAXREGNUM - 1 := 12;
  
  -- bank(13)       : DST_OFFSET (offset in bytes from beginning of physical buffer)
  constant DST_OFFSET_REG : integer range 0 to MAXREGNUM - 1 := 13;
  
  -- bank(14)       : RESERVED

  -- bank(15)       : RESERVED

  -- bank(16 to 28) : USR (user defined)

  -- bank(29)       : EXP_ADDR (bits 29:0 address an SRAM expanding the register bank)
  constant EXP_ADDR_REG : integer range 0 to MAXREGNUM - 1 := 29;
  constant EXT_BIT_R : integer range 0 to 31 := 30;
  constant EXT_BIT_W : integer range 0 to 31 := 31;

  -- bank(30)       : EXP_DI (data to be written to the expansion SRAM)
  constant EXP_DI_REG : integer range 0 to MAXREGNUM - 1 := 30;
  
  -- bank(31)       : EXP_DO (data read from the exansion SRAM)
  constant EXP_DO_REG : integer range 0 to MAXREGNUM - 1 := 31;

  -- Helper functions
  constant zero : std_logic_vector(31 downto 0) := (others => '0');
  constant one : std_logic_vector(31 downto 0) := x"00000001";
  constant fff : std_logic_vector(31 downto 0) := x"ffffffff";

  function right_shift (
    signal in_vect : std_logic_vector(31 downto 0);
    signal amount  : std_logic_vector(4 downto 0))
    return std_logic_vector;
  
  function left_shift (
    signal in_vect : std_logic_vector(31 downto 0);
    signal amount  : std_logic_vector(4 downto 0))
    return std_logic_vector;

end acctypes;

package body acctypes is

  function right_shift (
    signal in_vect : std_logic_vector(31 downto 0);
    signal amount  : std_logic_vector(4 downto 0))
    return std_logic_vector is
    variable after16  : std_logic_vector(31 downto 0);
    variable after8   : std_logic_vector(31 downto 0);
    variable after4   : std_logic_vector(31 downto 0);
    variable after2   : std_logic_vector(31 downto 0);
    variable after1   : std_logic_vector(31 downto 0);
  begin -- right_shift
    if amount(4) = '1' then
      after16 := zero(15 downto 0) & in_vect(31 downto 16);
    else
      after16 := in_vect;
    end if;
    if amount(3) = '1' then
      after8 := zero(7 downto 0) & after16(31 downto 8);
    else
      after8 := after16;
    end if;
    if amount(2) = '1' then
      after4 := zero(3 downto 0) & after8(31 downto 4);
    else
      after4 := after8;
    end if;
    if amount(1) = '1' then
      after2 := "00" & after4(31 downto 2);
    else
      after2 := after4;
    end if;
    if amount(0) = '1' then
      after1 := "0" & after2(31 downto 1);
    else
      after1 := after2;
    end if;
    return after1;
  end right_shift;

  function left_shift (
    signal in_vect    : std_logic_vector(31 downto 0);
    signal amount     : std_logic_vector(4 downto 0))
    return std_logic_vector is
    variable after16  : std_logic_vector(31 downto 0);
    variable after8   : std_logic_vector(31 downto 0);
    variable after4   : std_logic_vector(31 downto 0);
    variable after2   : std_logic_vector(31 downto 0);
    variable after1   : std_logic_vector(31 downto 0);
  begin
    if amount(4) = '1' then
      after16 :=  in_vect(15 downto 0) & zero(15 downto 0);
    else
      after16 := in_vect;
    end if;
    if amount(3) = '1' then
      after8 := after16(23 downto 0) & zero(7 downto 0);
    else
      after8 := after16;
    end if;
    if amount(2) = '1' then
      after4 := after8(27 downto 0) & zero(3 downto 0);
    else
      after4 := after8;
    end if;
    if amount(1) = '1' then
      after2 := after4(29 downto 0) & "00";
    else
      after2 := after4;
    end if;
    if amount(0) = '1' then
      after1 := after2(30 downto 0) & "0";
    else
      after1 := after2;
    end if;
    return after1;
  end left_shift;

end acctypes;