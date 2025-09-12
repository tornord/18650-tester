import eslint from "@eslint/js";
import parserTs from "@typescript-eslint/parser";
import stylisticTs from "@stylistic/eslint-plugin-ts";
import tseslint from "typescript-eslint";

export default [
  { ignores: ["dist/**/*", "lib/**/*", "storybook-static/**/*", "public/**/*", "ml/**/*", "scripts/CryptoJS.js"] },
  eslint.configs.recommended,
  ...tseslint.configs.recommended,
  {
    languageOptions: { sourceType: "module", parser: parserTs },
    plugins: { stylistic: stylisticTs },
    settings: {},
    rules: {
      "@typescript-eslint/no-loop-func": "error",
      "@typescript-eslint/no-shadow": "error",
      "@typescript-eslint/no-unused-vars": ["error", { argsIgnorePattern: "^_", varsIgnorePattern: "^_" }],
      camelcase: 1,
      eqeqeq: 2,
      "dot-notation": [2, { allowKeywords: true }],
      "new-cap": 2,
      "no-console": 2,
      "no-duplicate-imports": 1,
      "no-eval": 2,
      "no-var": 2,
      "prefer-const": ["error", { destructuring: "all" }],
      "prefer-template": 1,
      quotes: [1, "double"],
      semi: [2, "always"],
      "sort-imports": ["error", { ignoreCase: true, allowSeparatedGroups: true }],
      strict: [2, "global"],
    },
  },
  {
    files: ["src/**/*.test.ts", "common/**/*.test.ts", "scripts/**/*.test.ts", "src/**/*.test.tsx"],
    languageOptions: {
      globals: {
        expect: false,
        describe: false,
        it: false,
        afterEach: false,
        test: false,
        beforeAll: false,
        beforeEach: false,
        jest: false,
      },
    },
    rules: {},
  },
];
