import React from 'react';
import { Drawer, List, ListItem, ListItemButton, ListItemIcon, ListItemText, Toolbar, Divider, Box, Typography } from '@mui/material';
import DashboardIcon from '@mui/icons-material/Dashboard';
import CloudUploadIcon from '@mui/icons-material/CloudUpload';
import AnalyticsIcon from '@mui/icons-material/Analytics';
import { Link } from 'react-router';

const drawerWidth = 240;

const selectedIndex = 0;

interface SidebarProps {
    mobileOpen: boolean;
    handleDrawerToggle: () => void;
    handleDrawerChange: (index: number) => void;
}

interface DrawerItem {
    index: number;
    text: string;
    icon: React.ReactElement;
    link: string;
}

const drawerItems: DrawerItem[] = [
    {index: 0, text: 'Dashboard', icon: <DashboardIcon />, link: '/'},
    {index: 1, text: 'Upload', icon: <CloudUploadIcon />, link: '/upload'},
    {index: 2, text: 'Analytics', icon: <AnalyticsIcon />, link: '/analytics'},
]

export default function Sidebar({ mobileOpen, handleDrawerToggle, handleDrawerChange }: SidebarProps) {
  const drawerContent = (
    <div>
      <List>
        {drawerItems.map(item => (
          <Link to={item.link}>
            <ListItem key={item.text} disablePadding onClick={() => handleDrawerChange(item.index)} >
              <ListItemButton>
                <ListItemIcon>
                  {item.icon}
                </ListItemIcon>
                <ListItemText primary={item.text} />
              </ListItemButton>
            </ListItem>
          </Link>
        ))}
      </List>
    </div>
  );

  return (
    <Box component="nav" sx={{ width: { sm: drawerWidth }, flexShrink: { sm: 0 } }}>
      <Drawer
        variant="temporary"
        open={mobileOpen}
        onClose={handleDrawerToggle}
        sx={{
          display: { xs: 'block', sm: 'none' },
          '& .MuiDrawer-paper': { boxSizing: 'border-box', width: drawerWidth },
        }}
      >
        {drawerContent}
      </Drawer>
      
      <Drawer
        variant="permanent"
        sx={{
          display: { xs: 'none', sm: 'block' },
          '& .MuiDrawer-paper': { boxSizing: 'border-box', width: drawerWidth },
        }}
        open
      >
        {drawerContent}
      </Drawer>
    </Box>
  );
}