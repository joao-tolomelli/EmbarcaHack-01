export default {
  testEnvironment: 'node',
  transform: {},
  moduleNameMapper: {
    '^(\\.{1,2}/.*)\\.js$': '$1'
  },
  globalTeardown: './tests/setup/globalTeardown.js',
  globalSetup: './tests/setup/globalSetup.js'
};
