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
  signal shifter: std_logic_vector(9 downto 0);
  alias   irx: std_logic is usr(5);
  alias   idtr:std_logic is usr(4);
  signal internal_reset_q: std_logic;
  signal hclock: std_logic;
  signal data_queued: std_logic;
  signal internalcs:  std_logic;
  alias di: std_logic is gpio13;
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
  internalcs <= gpio4 when gpio16='0' else '1'; -- Ignore commands when SW not running.

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


  process(clk,internalcs)
  begin
    if internalcs='1' then
      shifter <= (others => '0');
    elsif rising_edge(clk) then
      if internal_reset_q='1' then
        shifter(8 downto 2) <= (others => '0');
        shifter(1) <= shifter(0);
        shifter(0) <= di;
      else
        shifter <= shifter(shifter'HIGH-1 downto 0) & di;
      end if;
    end if;
  end process;

  --clko <= shifter(8);
  clko <= hclock;

  process(clk,internalcs)
  begin
    if internalcs='1' then
      internal_reset_q<='0';
      data_queued<='0';
    elsif rising_edge(clk) then
      if internal_reset_q='1' then
        internal_reset_q<='0';
      else
        internal_reset_q <= shifter(8);
        if shifter(8)='1' then
          data_queued<='1';
        end if;
      end if;
      if hclock='1' then
        data_queued<='0';
      end if;
    end if;
  end process;

  process(clk,internalcs)
  begin
    if internalcs='1' then
      hclock<='0';
    elsif rising_edge(clk) then
      if shifter(8 downto 6)="001" then
        hclock<='0';
      else
        --if internal_reset_q='1' then
        if data_queued='1' and shifter(3)='1' and internal_reset_q='0' then
          hclock <= '1';
        end if;
      end if;
    end if;
  end process;

  process(clk)
  begin
    if rising_edge(clk) then
      if shifter(8 downto 6)="001" then
        rgb <= shifter(4 downto 0) & di;
      end if;
    end if;
  end process;

  process(gpio4)
  begin
    if rising_edge(gpio4) then
      if shifter(8)='0' then
        col <= shifter(3 downto 0);
      end if;
    end if;
  end process;

end Behavioral;

