import { Box, Card, Toolbar, Typography } from "@mui/material";

import List from '@mui/material/List';
import ListItem from '@mui/material/ListItem';

import {
    useQuery,
  useQueryClient,
} from '@tanstack/react-query'


const Home = () => {
    const queryClient = useQueryClient();

    const getFiles = async () => {
        const res: Response = await fetch('http://127.0.0.1:8000/files');

        // console.log(res)
        
        if (!res.ok) {
            throw new Error('Network response was not ok');
        }

        const data = await res.json()

        console.log(data)
        
        return data.files;
    }

    const { isPending, error, data } = useQuery({
        queryKey: ['files'],
        queryFn: getFiles
    })

    if (isPending) return <Typography>Loading...</Typography>

    if (error) return <Typography>An error has occurred: {error.message}</Typography>

    return (
        <>
            <Typography variant="h4" sx={{ fontWeight: 800 }}>
                Home
            </Typography>
            <Box sx={{ width: "100%", display: "grid", gridTemplateColumns: "auto auto auto" }}>
            <List dir="horizontal">
                { data.map((item: string) => (
                    <ListItem>
                        {item}
                    </ListItem>
                )) }
            </List>
        </Box>
        </>
    );
}

export default Home;