import pool from '../../src/config/db.js';

export default async () => {
  console.log('\nFinalizando conexão com MySQL...');
  await pool.end();
  await new Promise(res => setTimeout(res, 1000));
};
