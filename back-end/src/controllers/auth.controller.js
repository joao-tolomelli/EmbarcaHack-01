import jwt from 'jsonwebtoken';
import bcrypt from 'bcrypt';
import pool from '../config/db.js';

const saltRounds = 10;

function generateToken(user) {
  return jwt.sign(
    { id: user.id, email: user.email, profile: user.profile },
    process.env.JWT_SECRET,
    { expiresIn: '1h' }
  );
}


export async function login(req, res) {
  const { email, password } = req.body;

  try {
    const [rows] = await pool.query('SELECT * FROM users WHERE email = ?', [email]);

    if (rows.length === 0) {
      return res.status(401).json({ message: 'Credenciais inválidas' });
    }

    const user = rows[0];
    const match = await bcrypt.compare(password, user.password);

    if (!match) {
      return res.status(401).json({ message: 'Credenciais inválidas' });
    }

    const token = generateToken(user);
    res.json({ token });
  } catch (err) {
    console.error('Erro no login:', err);
    res.status(500).json({ message: 'Erro interno no servidor' });
  }
}

export async function register(req, res) {
  const { name, email, password, profile = 3 } = req.body;

  // Verifica se o perfil é válido (entre 0 e 3)
  // 0 = admin, 1 = medico, 2 = farmaceutico, 3 = familiar, 4 = paciente
  if (![0, 1, 2, 3, 4].includes(profile)) {
    return res.status(400).json({ message: 'Perfil de usuário inválido' });
  }

  try {
    const [existing] = await pool.query('SELECT id FROM users WHERE email = ?', [email]);

    if (existing.length > 0) {
      return res.status(400).json({ message: 'E-mail já registrado' });
    }

    const hashedPassword = await bcrypt.hash(password, saltRounds);

    await pool.query(
      'INSERT INTO users (name, email, password, profile) VALUES (?, ?, ?, ?)',
      [name, email, hashedPassword, profile]
    );

    res.status(201).json({ message: 'Usuário registrado com sucesso' });
  } catch (err) {
    console.error('Erro no registro:', err);
    res.status(500).json({ message: 'Erro interno no servidor' });
  }
}
