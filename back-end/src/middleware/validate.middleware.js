// src/middleware/validate.middleware.js
export function validate(schema) {
  return (req, res, next) => {
    const { error } = schema.validate(req.body, { abortEarly: false });

    if (error) {
      const details = error.details.map(err => err.message);
      return res.status(400).json({ message: 'Dados inválidos', errors: details });
    }

    next();
  };
}
