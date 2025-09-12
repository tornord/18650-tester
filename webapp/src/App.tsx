import { useEffect, useRef, useState } from "react";
import { css } from "@emotion/react";
import styled from "@emotion/styled";

import Battery, { BatteryData } from "./Battery";
import { secs } from "./time-helper";

/* eslint-disable  @typescript-eslint/no-explicit-any */
const { VITE_ARDUINO_IPS } = (import.meta as any).env; 
const FETCH_INTERVAL = 60000; // 60 seconds

const StyledApp = styled.div(
  () => css`
    width: 100%;
    height: 600px;
  `
);

const BDIDLE = {
  name: "A",
  state: "IDLE",
  elapsedTime: 1.85,
  voltage: 0.052,
  current: -0.0004,
  inputVoltage: 0.052,
  power: 0,
  internalResistance: -1,
};

const BDCHG = {
  name: "A",
  state: "CHG",
  elapsedTime: 27555.72,
  voltage: 4.108,
  current: 0.2742,
  inputVoltage: 4.135,
  power: 1.11,
  internalResistance: 0.0597,
  dischargeTotalTime: 8105.11,
  dischargeCapacity: 2887.6,
  dischargeEnergy: 10616.9,
  chargeTotalTime: 14326.34,
  chargeCapacity: 2247.25,
  chargeEnergy: 8562.9,
};

const BD3 = {
  name: "B",
  state: "NOINA",
  elapsedTime: 33523.51,
  voltage: 0,
  current: 0,
  inputVoltage: 0,
  power: 0,
  internalResistance: -1,
};

const BDERR = {
  name: "A",
  state: "ERR",
  elapsedTime: 1.36,
  voltage: 0.716,
  current: -0.0004,
  inputVoltage: 0.716,
  power: 0,
  internalResistance: -1,
};

const BDFIN = {
  name: "A",
  state: "FIN",
  elapsedTime: 29439.96,
  voltage: 4.168,
  current: -0.0007,
  inputVoltage: 4.168,
  power: 0,
  internalResistance: 0.1138,
  dischargeTotalTime: 8044.61,
  dischargeCapacity: 2558.5,
  dischargeEnergy: 9282,
  chargeTotalTime: 16291.84,
  chargeCapacity: 2526.59,
  chargeEnergy: 9790.6,
};

// @ts-expect-error no-unused-vars
const BDS = [BDCHG, BD3, BDIDLE, BDERR, BDFIN]; // eslint-disable-line @typescript-eslint/no-unused-vars

interface State {
  currTimestamp: number;
  currData: BatteryData[];
  prevTimestamp: number;
  prevData: BatteryData[];
}

function initState(): State {
  const t = secs();
  return {
    currTimestamp: t,
    currData: [],
    prevTimestamp: t,
    prevData: [],
  };
}

export function App() {
  const [timestamp, setTimestamp] = useState(secs());
  useEffect(() => {
    const interval = setInterval(() => {
      setTimestamp(secs());
    }, 1000);
    return () => clearInterval(interval);
  }, []);
  const urls = Object.fromEntries(VITE_ARDUINO_IPS.split(",").map((id: string) => [id, `http://192.168.1.${id}`]));

  const [batteries, setBatteries] = useState<State>(initState());
  const batteriesRef = useRef<State>(batteries);
  batteriesRef.current = batteries;
  useEffect(() => {
    let isMounted = true;
    const fetchAll = async () => {
      try {
        const results = await Promise.all(
          Object.entries(urls).map(([id, url]) =>
            fetch(url)
              .then((res) => res.json())
              .then((d) => {
                d.forEach((b: BatteryData) => {
                  b.id = `${id}-${b.name}`;
                  b.url = url;
                });
                return d;
              })
              .catch(() => [])
          )
        );
        // Flatten and filter out any non-array responses
        const allBatteries = results.flat().filter((b) => b && b.name);
        if (isMounted) {
          setBatteries({
            currTimestamp: secs(),
            currData: allBatteries,
            prevTimestamp: batteriesRef.current.currTimestamp,
            prevData: [...batteriesRef.current.currData],
          });
        }
      } catch {
        if (isMounted) setBatteries(initState());
      }
    };
    fetchAll();
    const interval = setInterval(fetchAll, FETCH_INTERVAL);
    return () => {
      isMounted = false;
      clearInterval(interval);
    };
  }, []);
  return (
    <StyledApp>
      <div style={{ display: "flex", flexWrap: "wrap", gap: 16, marginTop: 24 }}>
        {batteries.currData.map((b, i) => (
          <Battery
            key={b.name + i}
            timestamp={timestamp}
            currData={b}
            prevData={batteries.prevData.find((pb) => pb.id === b.id) ?? null}
            currTimestamp={batteries.currTimestamp}
            prevTimestamp={batteries.prevTimestamp}
          />
        ))}
      </div>
    </StyledApp>
  );
}
