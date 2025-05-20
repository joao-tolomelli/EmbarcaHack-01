const { Pool } = require("pg");
require("dotenv").config();

const dbURL = process.env.DB_URL;

const pool = new Pool({
  connectionString: dbURL,
  ssl: {
    rejectUnauthorized: false 
  }
});

async function query(queryString, params, callback) {
  return pool.query(queryString, params, callback);
}

module.exports = { query };
