// vite.config.js
import { fileURLToPath, URL } from "node:url";

import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";

// https://vite.dev/config/
export default defineConfig({
  plugins: [vue()],

  resolve: {
    alias: {
      "@": fileURLToPath(new URL("./src", import.meta.url)),
    },
  },

  define: {
    global: "globalThis",
  },

  server: {
    host: "0.0.0.0",
    port: 5173,
    strictPort: true,

    allowedHosts: ["evader84.picp.io"],

    proxy: {
      "/api": {
        target: "http://127.0.0.1:10000",
        changeOrigin: true,
        ws: true,
      },
    },
  },
});