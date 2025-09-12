import { useState } from "react";

import styled from "@emotion/styled";

let MAX_SLOPE = 0;

const postPrint = async (capacity: number, internalResistance: number) => {
  await fetch("http://localhost:5001", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      capacity,
      internalResistance,
    }),
  });
};

const Sub = styled.span`
  font-size: 0.8em;
  vertical-align: sub;
`;

export interface BatteryData {
  id: string;
  url: string;
  name: string;
  state: string;
  elapsedTime: number;
  voltage: number;
  current: number;
  inputVoltage: number;
  power: number;
  internalResistance: number;
  waitingToDischargeTime?: number;
  dischargeTotalTime?: number;
  dischargeCapacity?: number;
  dischargeEnergy?: number;
  waitingToChargeTime?: number;
  chargeTotalTime?: number;
  chargeCapacity?: number;
  chargeEnergy?: number;
}

interface BatteryProps {
  timestamp: number;
  currData: BatteryData;
  currTimestamp: number;
  prevData: BatteryData | null;
  prevTimestamp: number;
}

const PrintSvg = () => (
  <svg
    xmlns="http://www.w3.org/2000/svg"
    xmlnsXlink="http://www.w3.org/1999/xlink"
    version="1.1"
    id="Uploaded to svgrepo.com"
    width={24}
    height={24}
    viewBox="0 0 32 32"
    xmlSpace="preserve"
    fill="#000000"
  >
    <g id="SVGRepo_bgCarrier" strokeWidth="0" />
    <g id="SVGRepo_tracerCarrier" strokeLinecap="round" strokeLinejoin="round" />
    <g id="SVGRepo_iconCarrier">
      <style type="text/css">
        {`
          .cubies_twee { fill: #67625D; }
          .cubies_een { fill: #4C4842; }
          .cubies_twaalf { fill: #FFF2DF; }
          .cubies_vijftien { fill: #D1DE8B; }
          .cubies_vier { fill: #A5A29C; }
          .cubies_drie { fill: #837F79; }
          .st0 { fill: #F2C99E; }
          .st1 { fill: #F9E0BD; }
          .st2 { fill: #65C3AB; }
          .st3 { fill: #725A48; }
          .st4 { fill: #8E7866; }
          .st5 { fill: #D97360; }
          .st6 { fill: #98D3BC; }
          .st7 { fill: #C9483A; }
          .st8 { fill: #CCE2CD; }
          .st9 { fill: #EDB57E; }
          .st10 { fill: #EC9B5A; }
          .st11 { fill: #C9C6C0; }
          .st12 { fill: #EDEAE5; }
          .st13 { fill: #A4C83F; }
          .st14 { fill: #BCD269; }
          .st15 { fill: #E69D8A; }
          .st16 { fill: #E3D4C0; }
          .st17 { fill: #C6B5A2; }
          .st18 { fill: #2EB39A; }
          .st19 { fill: #AB9784; }
        `}
      </style>
      <g>
        <path className="cubies_drie" d="M32,20H0V3c0-1.657,1.343-3,3-3h26c1.657,0,3,1.343,3,3V20z" />
        <path className="cubies_vier" d="M30,20H0V3c0-1.657,1.343-3,3-3h24c1.657,0,3,1.343,3,3V20z" />
        <path className="cubies_een" d="M29,32H3c-1.657,0-3-1.343-3-3v-9h32v9C32,30.657,30.657,32,29,32z" />
        <path className="cubies_twee" d="M0,20h30v9c0,1.657-1.343,3-3,3H3c-1.657,0-3-1.343-3-3L0,20z" />
        <circle className="cubies_vijftien" cx="27" cy="3" r="1" />
        <path
          className="cubies_twee"
          d="M23,3c0-0.552,0.448-1,1-1s1,0.448,1,1c0,0.552-0.448,1-1,1S23,3.552,23,3z M27,16H3 c-0.552,0-1,0.448-1,1c0,0.552,0.448,1,1,1h24c0.552,0,1-0.448,1-1C28,16.448,27.552,16,27,16z"
        />
        <path className="cubies_twaalf" d="M26,29H4c-0.552,0-1-0.448-1-1V16h24v12C27,28.552,26.552,29,26,29z" />
        <path
          className="cubies_drie"
          d="M21,21H9c-0.552,0-1-0.447-1-1s0.448-1,1-1h12c0.552,0,1,0.447,1,1S21.552,21,21,21z M22,24 c0-0.553-0.448-1-1-1H9c-0.552,0-1,0.447-1,1s0.448,1,1,1h12C21.552,25,22,24.553,22,24z M9,17h12c0.552,0,1-0.447,1-1H8 C8,16.553,8.448,17,9,17z"
        />
      </g>
    </g>
  </svg>
);

const StyledDiv = styled.div`
  border: 1px solid #ccc;
  border-radius: 8px;
  padding: 16px;
  max-width: 400px;
  background: #f9f9f9;

  h2 {
    a {
      text-decoration: none;
      color: inherit;
    }
  }
`;

const Battery: React.FC<BatteryProps> = ({ timestamp, currData, prevData, currTimestamp, prevTimestamp }) => {
  function extrapolateValue(key: string): number {
    const { state } = currData;
    /* eslint-disable  @typescript-eslint/no-explicit-any */
    const currValue = (currData as any)[key];
    if (!["ICHG", "CHG", "DCHG", "WCHG", "WDCHG"].includes(currData.state)) return currValue;
    if (
      key === "elapsedTime" ||
      (state === "DCHG" && key === "dischargeTotalTime") ||
      (state === "CHG" && key === "chargeTotalTime")
    ) {
      return currValue + (timestamp - currTimestamp);
    }
    if (!["ICHG", "CHG", "DCHG", "WCHG", "WDCHG"].includes(currData.state)) return currValue;
    if (
      (state === "WDCHG" && key === "waitingToDischargeTime") ||
      (state === "WCHG" && key === "waitingToChargeTime")
    ) {
      return Math.max(0, currValue - (timestamp - currTimestamp));
    }
    if (prevData === null) return currValue;
    if (currData.state !== prevData.state) return currValue;
    const prevValue = (prevData as any)[key];
    const slope = (prevValue - currValue) / (prevTimestamp - currTimestamp);
    if (Math.abs(slope) > Math.abs(MAX_SLOPE)) {
      MAX_SLOPE = slope;
      console.log("New max slope for", key, ":", MAX_SLOPE); // eslint-disable-line no-console
    }
    return currValue + slope * (timestamp - currTimestamp);
  }

  const formatTime = (seconds: number) => {
    const h = Math.floor(seconds / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    const s = Math.floor(seconds % 60);
    return [h, m, s].map((v) => v.toString().padStart(2, "0")).join(":");
  };
  const [loading, setLoading] = useState(false);
  const handlePrint = async () => {
    console.log("Printing", currData.chargeCapacity, currData.internalResistance); // eslint-disable-line no-console
    if (!currData.dischargeCapacity || !currData.internalResistance) return;
    setLoading(true);
    await postPrint(
      Number((currData.dischargeCapacity / 1000).toFixed(1)),
      Number(currData.internalResistance.toFixed(3))
    );
    setLoading(false);
  };
  const voltage = extrapolateValue("voltage");
  const current = extrapolateValue("current");
  const power = Math.abs(voltage * current);
  return (
    <StyledDiv>
      <h2>
        <a href={currData.url} target="_blank" rel="noopener noreferrer">
          <span>{currData.id ? currData.id : currData.name}</span>
          {currData.state !== "NOINA" ? ` - ${currData.state}` : ""}
        </a>
      </h2>
      {["IDLE", "NOINA", "UKWN"].includes(currData.state) ? null : (
        <div style={{ marginBottom: 8 }}>
          <strong>Elapsed Time: </strong>
          {formatTime(extrapolateValue("elapsedTime"))}
        </div>
      )}
      <div style={{ marginBottom: 8 }}>
        <strong>U = </strong> {voltage.toFixed(3)} V
      </div>
      <div style={{ marginBottom: 8 }}>
        <strong>I = </strong> {(1000 * current).toFixed(1)} mA
      </div>
      <div style={{ marginBottom: 8 }}>
        <strong>
          U<Sub>IN</Sub> ={" "}
        </strong>{" "}
        {currData.inputVoltage.toFixed(3)} V
      </div>
      <div style={{ marginBottom: 8 }}>
        <strong>P = </strong> {power.toFixed(2)} W
      </div>
      {currData.internalResistance !== -1 && (
        <div style={{ marginBottom: 8 }}>
          <strong>
            R<Sub>INT</Sub> ={" "}
          </strong>{" "}
          {(100 * currData.internalResistance).toFixed(1)} cΩ
        </div>
      )}
      {currData.waitingToDischargeTime && (
        <div style={{ marginBottom: 8 }}>
          <strong>Discharge starts in:</strong> {formatTime(extrapolateValue("waitingToDischargeTime"))}
        </div>
      )}
      {currData.dischargeTotalTime && (
        <div style={{ marginBottom: 8 }}>
          <strong>Discharge Total Time:</strong> {formatTime(extrapolateValue("dischargeTotalTime"))}
        </div>
      )}
      {currData.dischargeCapacity && (
        <div style={{ marginBottom: 8 }}>
          <strong>Discharge Capacity:</strong> {(extrapolateValue("dischargeCapacity") / 100).toFixed(2)} dAh
        </div>
      )}
      {currData.dischargeEnergy && (
        <div style={{ marginBottom: 8 }}>
          <strong>Discharge Energy:</strong> {(extrapolateValue("dischargeEnergy") / 1000).toFixed(2)} Wh
        </div>
      )}
      {currData.waitingToChargeTime && (
        <div style={{ marginBottom: 8 }}>
          <strong>Discharge starts in:</strong> {formatTime(extrapolateValue("waitingToChargeTime"))}
        </div>
      )}
      {currData.chargeTotalTime && (
        <div style={{ marginBottom: 8 }}>
          <strong>Charge Total Time:</strong> {formatTime(extrapolateValue("chargeTotalTime"))}
        </div>
      )}
      {currData.chargeCapacity && (
        <div style={{ marginBottom: 8 }}>
          <strong>Charge Capacity:</strong> {(extrapolateValue("chargeCapacity") / 100).toFixed(2)} dAh
        </div>
      )}
      {currData.chargeEnergy && (
        <div style={{ marginBottom: 8 }}>
          <strong>Charge Energy:</strong> {(extrapolateValue("chargeEnergy") / 1000).toFixed(2)} Wh
        </div>
      )}
      {["WCHG", "CHG", "FIN"].includes(currData.state) && (
        <button
          onClick={handlePrint}
          disabled={loading}
          style={{
            marginTop: 12,
            border: "none",
            background: "#eee",
            padding: 0,
            borderRadius: 4,
            cursor: loading ? "not-allowed" : "pointer",
            display: "flex",
            alignItems: "center",
            justifyContent: "center",
            width: 40,
            height: 40,
          }}
          title={loading ? "Printing..." : "Print"}
        >
          <span style={{ opacity: loading ? 0.5 : 1 }}>
            <PrintSvg />
          </span>
        </button>
      )}
    </StyledDiv>
  );
};

export default Battery;
