import { css, Global } from "@emotion/react";
import ReactDOM from "react-dom/client";

import { App } from "./App";

ReactDOM.createRoot(document.getElementById("root") as Element).render(
  <>
    <Global
      styles={css`
        :root {
          font-family: Inter, Avenir, Helvetica, Arial, sans-serif;
          font-size: 14px;
          line-height: 16px;
          font-weight: 400;
        }

        body {
          margin: 12px;
          background-color: #181818;
        }
      `}
    />
    <App />
  </>
);
