/* eslint-disable no-console */
/* eslint-disable  @typescript-eslint/no-explicit-any */

import fs from "node:fs";

import fetch from "node-fetch";

const sum = (vs: number[]) => vs.reduce((a, b) => a + b, 0);

async function main() {
  let batteries;
  try {
    const res = await fetch("http://192.168.1.131/");
    if (res.ok) {
      batteries = await res.json();
      console.log("Fetched data from http://192.168.1.131/");
      fs.writeFileSync("scripts/data.json", JSON.stringify(batteries, null, 2));
    } else {
      throw new Error("Fetch failed");
    }
  } catch {
    const raw = fs.readFileSync("scripts/data.json", "utf-8");
    batteries = JSON.parse(raw);
    console.log("Read data from scripts/data.json");
  }
  const b1 = batteries[0];
  const ps = b1.measurePoints;
  console.log("Number of measurements:", ps.length);

  const times = ps.map((p: any) => p.time);
  const vs = ps.map((p: any) => p.voltage);
  const cs = ps.map((p: any) => p.current);

  const ms = cs.map((c: number, i: number, a: number[]) => {
    const c0 = i === 0 ? c : a[i - 1];
    const dc = Math.abs((c + c0) / 2);
    const dt = (times[i] - (i === 0 ? 0 : times[i - 1])) / 3600;
    return dc * dt;
  });
  const voltDiffs = vs.map((v: number, i: number, a: number[]) => {
    const v0 = i === 0 ? v : a[i - 1];
    return v - v0;
  });

  console.log("times", times);
  console.log("capacity", sum(ms));
  console.log("voltage diffs", voltDiffs);
}

await main();
