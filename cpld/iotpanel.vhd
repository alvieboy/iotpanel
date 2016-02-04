library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity iotpanel is
  port (
    clk:  in std_logic;
    cs:   inout std_logic;
    --di:   in std_logic;

    rgb:  out std_logic_vector(5 downto 0);
    col:  out std_logic_vector(3 downto 0);
    stb:  out std_logic;
    clko:  out std_logic;

    --idtr:  in std_logic;
    gpio13: in std_logic;
    gpio14: in std_logic;
    espreset: out std_logic;
    espen:  inout std_logic;
    esptx:  in std_logic;
    esprx:  out std_logic;
    gpio2: inout std_logic;
    gpio0: inout std_logic;
    gpio5: inout std_logic;
    gpio4: inout std_logic;
    --espchpd: out std_logic;
    gpio16: inout std_logic;
    gpio12: inout std_logic;
    oe: inout std_logic;
    panelen: inout std_logic;

    usr:    inout std_logic_vector(6 downto 3);
    iusr:   in std_logic
  );
end iotpanel;

architecture Behavioral of iotpanel is
  signal  shifter: std_logic_vector(7 downto 0);
  alias   irx: std_logic is usr(5);
  alias   idtr:std_logic is usr(4);
  signal  internal_cs: std_logic;
  signal  hclock: std_logic;
  signal  external_cs:  std_logic;
  alias   di: std_logic is gpio13;
begin

  -- CH_PD          Pull-up
  -- GPIO16/RST     Pull-up
  -- GPIO15         Pull-down
  -- GPIO2          Pull-up
  -- GPIO0          Pull-up for normal or pull-down for bootloader mode.

  gpio2 <= 'Z'; -- External pullup
  --gpio0 <= not irx; -- RX from external programmer.
  gpio0 <= usr(3);

  gpio16 <= 'Z'; -- External pullup
  gpio12 <= 'Z';
  gpio5  <= 'Z';


  cs <= '0' when gpio16='1' else 'Z';  -- Startup/Working CS mode.
  -- CS is now gpio4
  external_cs <= gpio4 when gpio16='0' else '1'; -- Ignore commands when SW not running.

  --usr(3) <= gpio4;--'Z';

  esprx <= usr(5);
  usr(6) <= esptx;

  oe <= gpio5 when gpio16='0' else '1';

  panelen <= 'Z';  -- not used yet
  espen <= 'Z';    -- not used yet

  espreset<='0' when idtr='1' else '1';

  --espen<='0' when idtr='0' and irx='1' else '1';

  stb <= gpio12;
  --stb <= clk;
  --clko <= gpio14;


  process(clk)
  begin
    if falling_edge(clk) then
      internal_cs<=shifter(7);
    end if;
  end process;

  process(clk,external_cs)
  begin
    if external_cs='1' then
      shifter <= "00000001";
    elsif falling_edge(clk) then
      if shifter( shifter'HIGH )='1' then
        shifter <= "00000001";
      else
        shifter <= shifter(shifter'HIGH-1 downto 0) & di;
      end if;

    end if;
  end process;

  clko <= hclock;

  process(clk,external_cs)
  begin
    if external_cs='1' then
      hclock<='0';
    elsif falling_edge(clk) then
      if internal_cs='1' then
        hclock<='0';
      else
        if shifter(4 downto 3)="11" then
          hclock<='1';
        end if;
      end if;
    end if;
  end process;

  process(clk)
  begin
    if falling_edge(clk) then
      if shifter(7 downto 6)="11" then
        rgb <= shifter(4 downto 0) & di;
      end if;
      if shifter(7 downto 6)="10" then
        col <= shifter(3 downto 0);--// & di;
      end if;
    end if;
  end process;

end Behavioral;

