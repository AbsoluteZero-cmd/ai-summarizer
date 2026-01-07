import React from 'react';
import DashboardLayout from './components/DashboardLayout';
import { Typography } from '@mui/material';
import { Route, Routes } from 'react-router';
import Upload from './pages/Upload';
import Analytics from './pages/Analytics';
import Home from './pages/Home';

export default function App() {
  return (
    <DashboardLayout>
      <Routes>
        <Route path='/' element={<Home />} />
        <Route path='/upload' element={<Upload />} />
        <Route path='/analytics' element={<Analytics />} />
      </Routes>
    </DashboardLayout>
  );
}