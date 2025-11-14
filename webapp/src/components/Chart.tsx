import React from "react";
import styled from "@emotion/styled";

export interface ChartProps {
  points: Array<{ x: number; y: number }>;
  width?: number;
  height?: number;
  color?: string;
  strokeWidth?: number;
  margin?: number;
}

const Svg = styled.svg`
  width: 100%;
  height: 100%;
  display: block;

  background-color: #222;
`;

export const Chart: React.FC<ChartProps> = ({
  points,
  width = 300,
  height = 120,
  color = "rgb(98 175 113)",
  strokeWidth = 2,
  margin = 10,
}) => {
  if (!points || points.length < 2) return null;

  // Find min/max for scaling
  const minX = Math.min(...points.map((p) => p.x));
  const maxX = Math.max(...points.map((p) => p.x));
  const minY = Math.min(...points.map((p) => p.y));
  const maxY = Math.max(...points.map((p) => p.y));

  // Map points to SVG coordinates
  const mapX = (x: number) => ((x - minX) / (maxX - minX || 1)) * (width - 2 * margin) + margin;
  const mapY = (y: number) => height - (((y - minY) / (maxY - minY || 1)) * (height - 2 * margin) + margin);

  const pathData = points
    .map((p, i) => `${i === 0 ? "M" : "L"}${mapX(p.x)},${mapY(p.y)}`)
    .join(" ");

  return (
    <Svg viewBox={`0 0 ${width} ${height}`} width={width} height={height}>
      <path d={pathData} fill="none" stroke={color} strokeWidth={strokeWidth} />
    </Svg>
  );
};

export default Chart;
