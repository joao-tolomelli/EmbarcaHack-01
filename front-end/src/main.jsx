import { PrimeReactProvider } from "primereact/api";
import { createRoot } from "react-dom/client";

import "./index.css";
import 'primeicons/primeicons.css'; // Esta linha é ESSENCIAL para os ícones aparecerem

import App from "./App.jsx";

createRoot(document.getElementById("root")).render(
  <PrimeReactProvider>
    <App />
  </PrimeReactProvider>
);
