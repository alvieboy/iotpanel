--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   11:05:08 03/28/2015
-- Design Name:   
-- Module Name:   /home/alvieboy/otpanel/cpld/tb.vhd
-- Project Name:  cpld
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: top
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
library work;
use work.all;

ENTITY tb IS
END tb;
 
ARCHITECTURE behavior OF tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)

   --Inputs
   signal clk : std_logic := '0';
   signal cs : std_logic := '0';
   signal di : std_logic := '0';
   --signal idtr : std_logic := '0';
   signal gpio13 : std_logic := '0';
   signal gpio14 : std_logic := '0';
   signal esptx : std_logic := '0';

	--BiDirs
   signal gpio2 : std_logic;
   signal gpio0 : std_logic;
   signal gpio5 : std_logic;
   signal gpio4 : std_logic;
   signal gpio16 : std_logic;
   signal gpio12 : std_logic;
   signal usr : std_logic_vector(6 downto 3);

 	--Outputs
   signal rgb : std_logic_vector(5 downto 0);
   signal col : std_logic_vector(3 downto 0);
   signal stb : std_logic;
   signal clko : std_logic;
   signal espreset : std_logic;
   signal espen : std_logic;
   signal esprx : std_logic;
   --signal espchpd : std_logic;
   signal oe : std_logic;
   signal panelen : std_logic;

   -- Clock period definitions
   constant clk_period : time := 10 ns;
--   constant clko_period : time := 10 ns;

   alias   idtr:std_logic is usr(4);

BEGIN

  usr(4) <= '1';
  gpio16 <= '0';

	-- Instantiate the Unit Under Test (UUT)
   uut: entity work.iotpanel PORT MAP (
          clk => clk,
          cs => cs,
          --di => di,
          rgb => rgb,
          col => col,
          stb => stb,
          clko => clko,
          --idtr => idtr,
          gpio13 => di,
          gpio14 => gpio14,
          espreset => espreset,
          espen => espen,
          esptx => esptx,
          esprx => esprx,
          gpio2 => gpio2,
          gpio0 => gpio0,
          gpio5 => gpio5,
          gpio4 => gpio4,
          --espchpd => espchpd,
          gpio16 => gpio16,
          gpio12 => gpio13,--gpio12,
          oe => oe,
          panelen => panelen,
          usr => usr,
          iusr => 'X'
        );

   -- Stimulus process
   stim_proc: process
    procedure transfer(data: in std_logic_vector; deassert: in boolean) is
      variable w,i: natural;
      variable d: std_logic_vector(data'high downto data'low);
    begin
      w := data'length;
      d := data;
      wait for clk_period/2;
      cs <= '0';
      wait for clk_period/2;
      l1: for i in 0 to w-1 loop
        -- Setup data.
        di <= d(data'high);
        wait for clk_period/2;
        clk <= '1';
        wait for clk_period/2;
        clk <= '0';
        d := d(data'high-1 downto 0) & 'X';
      end loop;
      wait for clk_period/2;
      if (deassert) then cs<='1'; end if;
    end procedure;
   begin		
      -- hold reset state for 100 ns.
      cs <= '1';
      wait for 100 ns;	

      transfer("11" & "010"&"010"&'X', false);
      transfer("11" & "111"&"111"&'X', false);
      transfer("11" & "000"&"000"&'X', false);
      transfer("10" & "XXX"&"XXX"&'X', true);

      transfer("00" & "0001111", true);

      wait for 100 ns;
      wait;
   end process;

END;
