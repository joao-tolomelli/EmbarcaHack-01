const { query } = require("./db");

async function syncMedicinesTable() {
  try {
    const checkTable = await query(`
      SELECT EXISTS (
        SELECT FROM information_schema.tables 
        WHERE table_name = 'medicines'
      );
    `);

    const tableExists = checkTable.rows[0].exists;

    if (tableExists) {
      console.log("A tabela 'medicines' já foi criada anteriormente.");
      return;
    }

    await query(`
      CREATE TABLE medicines (
        id SERIAL PRIMARY KEY,
        name TEXT NOT NULL,
        color TEXT NOT NULL,
        time TIME NOT NULL,
        status TEXT NOT NULL
      );
    `);

    console.log("Tabela 'medicines' criada com sucesso.");
  } catch (error) {
    console.error("Erro ao sincronizar a tabela 'medicines':", error);
    throw error;
  }
}

module.exports = { syncMedicinesTable };
