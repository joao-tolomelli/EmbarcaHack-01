// src/validators/auth.validator.js
import Joi from 'joi';

export const registerSchema = Joi.object({
  name: Joi.string().min(2).max(100).required(),
  email: Joi.string().email().required(),
  password: Joi.string().min(6).max(100).required(),
  profile: Joi.number().valid(0, 1, 2, 3).optional() // perfil padrão será 3 se não enviado
});

export const loginSchema = Joi.object({
  email: Joi.string().email().required(),
  password: Joi.string().required()
});
